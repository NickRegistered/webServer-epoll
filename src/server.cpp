#include "server.h"
Server::Server(const string root,const string IP,\
               const unsigned short port):\
    root(root),ipAddr(IP),port(port)
{

}

int Server::initServer()
{
    int ret,flag;
    srvFd = socket(AF_INET,SOCK_STREAM,0);
    if(srvFd == -1){
        sendMsg("Create server socket failed!\n");
        return -1;
    }
    sendMsg("Create server socket OK!\n");
    fcntl(srvFd,F_SETFL,O_NONBLOCK);
    flag = 1;

    /*set address reuse*/
    setsockopt(srvFd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));

    /*set server's address & port*/
    struct sockaddr_in srvAddr;
    memset(&srvAddr,0,sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    srvAddr.sin_port = htons(port);
    bind(srvFd,(sockaddr*)&srvAddr,sizeof(srvAddr));

    if((epfd = epoll_create1(EPOLL_CLOEXEC)) == -1){
        sendMsg("epoll create failed!\n");
        return -1;
    }

    sendMsg("Create epoll OK!\n");
    if((ret = listen(srvFd,MAX_EVENT))==-1){
        sendMsg("Listen failed!\n");
        return -1;
    }
    sendMsg("Server listen startup OK!\n");
    ev.data.fd = srvFd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,srvFd,&ev);
    sendMsg("Server startup OK!\n\n");
    return 0;
}

int Server::acceptConnection(){
    nAddrLen = sizeof(clientAddr);
    int clientFd = accept(srvFd,(sockaddr*)&clientAddr,&nAddrLen);
    if(clientFd == -1){
        sendMsg("accept failed!\n");
        return -1;
    }

    char clientIp[64] = {0};
    sendMsg("New Client IP: "+QString::fromLocal8Bit(inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,sizeof(clientIp)))
            +"  Port: " + QString::number(ntohs(clientAddr.sin_port)) + "\n");

    int ret = fcntl(clientFd,F_GETFL);
    ret |= O_NONBLOCK;
    fcntl(clientFd,F_SETFL,ret);

    ev.data.fd = clientFd;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;//client socket is available for read and write
    epoll_ctl(epfd,EPOLL_CTL_ADD,clientFd,&ev);
    sendMsg("\nOne new session is established!\n");
    return 0;
}

int Server::receiveRequestion(int clientFd){
    char clientIp[64] = {0};
    ssize_t len = recv(clientFd,recvBuff,BUFFER_SIZE,0);
    if(len == 0){
        sendMsg("one connectionclosed,IP: "\
                +QString::fromLocal8Bit(inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,sizeof(clientIp)))
                 +"  Port: " + QString::number(ntohs(clientAddr.sin_port)) + "\n\n");
        epoll_ctl(epfd,EPOLL_CTL_DEL,clientFd,nullptr);
        return -1;
    }
    else if(len < 0) {
        sendMsg("\nreceive data from client failed! errno:"+QString::number(errno) +\
                "IP: "+QString::fromLocal8Bit(inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,sizeof(clientIp)))
                +"  Port: " + QString::number(ntohs(clientAddr.sin_port)) + "\n\n");
        epoll_ctl(epfd,EPOLL_CTL_DEL,clientFd,nullptr);
        return -1;
    }
    sendMsg("receive data from IP: "\
            +QString::fromLocal8Bit(inet_ntop(AF_INET,&clientAddr.sin_addr.s_addr,clientIp,sizeof(clientIp)))
            +"  Port: " + QString::number(ntohs(clientAddr.sin_port)) + "\n\n");
    Requestion::resolve(recvBuff);
    sendMsg("Requestion Method:"+QString::fromLocal8Bit(Requestion::Method) + "\n"+\
            "Requestion URL" + QString::fromLocal8Bit(Requestion::URL)+"\n");
    return 0;
}


int Server::reseponseRequestion(int clientFd){
    int fd;
    size_t ret,sum;
    if((ret = Response::Header(Requestion::URL+1,Requestion::Extn,sendBuff))== (~0uL)){
        sendMsg("open " + QString::fromLocal8Bit(Requestion::URL+1) + " failed\n");
        return -1;
    }
    send(clientFd,sendBuff,ret,0);
    sendMsg("sending file: " + QString::fromLocal8Bit(Requestion::URL) + "\n");
    fd = open(Requestion::URL+1,O_RDONLY);
    sum = 0;
    while(ret = read(fd,sendBuff,BUFFER_SIZE)){
        send(clientFd,sendBuff,ret,0);
        sum += ret;
    }
    sendMsg("file sending succeeded!\n\n");
#ifdef DEBUG_ON
    cout<<Requestion::URL<<" "<<sum<<"Bytes\n";
#endif
    close(fd);
    return 0;
}

void Server::run(){
    int ret;
    string filename;
    working = ((ret = initServer()) != -1);
    chdir(root.c_str());
    while(working){
        trigger_num = epoll_wait(epfd,events,MAX_EVENT,500);
        for(int i=0;i<trigger_num;++i){
            int curFd = events[i].data.fd;
            if(curFd == srvFd){//new connections
                if((ret =(acceptConnection()) == -1))continue;
            }
            else{//receive data from clinents
                filename = root;
                //curFd is available for read
                if((events[i].events&EPOLLIN) &&(ret = receiveRequestion(curFd)) != -1){
                    reseponseRequestion(curFd);
                }
            }
        }
    }
    return;
}

Server::~Server(){
    epoll_ctl(epfd,EPOLL_CTL_DEL,srvFd,nullptr);
    close(epfd);
}
