#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDir>
#include <QDialog>
#include <QtWidgets>
#include "server.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    Server *epServer;
    QFile logfile;
    QTextStream fout;
    void closeEvent(QCloseEvent* event);

private slots:
    void showMsg(QString);
    void on_btn_Browse_clicked();
    void on_btn_Start_clicked(bool checked);

};

#endif // WIDGET_H
