#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

void QStr2str(const QString&,string&);

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("epoll-web-server");
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b"); //IP的正则表达式
    ui->lineEdit_IP->setValidator(new QRegExpValidator(rx,this));
    ui->lineEdit_IP->setText("127.0.0.1");
    ui->lineEdit_Port->setText("80");
    logfile.setFileName("logfile");
    logfile.open(QIODevice::WriteOnly|QIODevice::Append);
    fout.setDevice(&logfile);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::showMsg(QString QMsg){
    ui->textBrowser->insertPlainText(QMsg);
    ui->textBrowser->moveCursor(QTextCursor::End);
    fout << QMsg;
    qDebug()<<QMsg;
}

void Widget::on_btn_Browse_clicked()
{
    QString directory = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,"Find Files",QDir::homePath()));
    if(!directory.isEmpty()){
        if(ui->directoryBox->findText(directory)== -1)
            ui->directoryBox->addItem(directory);
        ui->directoryBox->setCurrentIndex(ui->directoryBox->findText(directory));
    }
}

void Widget::on_btn_Start_clicked(bool checked)
{
    if(checked){
        string root;
        string ip;
        unsigned short port;
        QStr2str(ui->directoryBox->currentText(),root);
        QStr2str(ui->lineEdit_IP->text(),ip);
        port = ui->lineEdit_Port->text().toUShort();
        epServer = new Server(root,ip,port);
        epServer->working = true;
        connect(epServer,SIGNAL(sendMsg(QString)),this,SLOT(showMsg(QString)));
        epServer->start();
        qDebug()<<"Server started!";
        ui->btn_Start->setText("Stop");
        ui->btn_Browse->hide();
    }
    else{
        epServer->working = false;
        epServer->wait();
        delete  epServer;
        ui->btn_Start->setText("Start");
        ui->btn_Browse->show();
    }
}

void Widget::closeEvent(QCloseEvent *event){
    int ret;
    if(ui->btn_Start->isChecked()){
        ret = QMessageBox::warning(this,"Server is still working","Are you sure to exit?",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
        if(ret == QMessageBox::Yes){
            on_btn_Start_clicked(false);
            event->accept();
        }
        else event->ignore();
    }
    else event->accept();
}

void QStr2str(const QString& Qstr,string& str){
    str = string((const char*)(Qstr.toLocal8Bit()));
}
