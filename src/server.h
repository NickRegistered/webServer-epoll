#ifndef SERVER_H
#define SERVER_H

#include <QThread>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "requestion.h"
#include "response.h"
using namespace std;
const int MAX_EVENT = 20;
const int BUFFER_SIZE = 1024;
class Server:public QThread
{
    Q_OBJECT
public:
    volatile bool working;

    Server();
    Server(const string root,const string IP,const unsigned short port);
    ~Server();
protected:
    void run();
private:
    const string root;
    const string ipAddr;
    const unsigned short port;
    char recvBuff[BUFFER_SIZE];
    char sendBuff[BUFFER_SIZE];

    int srvFd,epfd;
    struct epoll_event ev,events[MAX_EVENT];

    int trigger_num;
    struct sockaddr_in clientAddr;
    socklen_t nAddrLen;

private:
    int initServer();
    int acceptConnection();
    int receiveRequestion(int clientFd);
    int reseponseRequestion(int clientFd);

signals:
    void sendMsg(QString msg);
};

#endif // SERVER_H
