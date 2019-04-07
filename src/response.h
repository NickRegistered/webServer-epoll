#ifndef RESPONSE_H
#define RESPONSE_H

#include <sys/stat.h>
#include <string.h>
#include <map>
#include <iostream>
using namespace std;
class Response {
    static char Stat[32];		//状态行
    static char CntType[32];	//Content-Type;
    static char CntLen[32];	//Content-Length
public:
    Response();
    static size_t Header(const char*filenamefilename,char* Extn,char *sendBuff);//根据文件信息组织响应报文，写入buff中
};
#endif // RESPONSE_H
