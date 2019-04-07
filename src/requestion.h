#ifndef REQUESTION_H
#define REQUESTION_H
#include <string.h>

#ifdef DEBUG_ON
#include<iostream>
using namespace std;
#endif

class Requestion{
private:
    Requestion();
public:
    static char Method[8]; //请求方法
    static char Extn[8];   //请求文件扩展名
    static char URL[64];  //请求文件路径
public:
    static void resolve(char *req);

};
#endif // REQUESTION_H
