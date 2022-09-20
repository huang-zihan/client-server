#ifndef __DEF__
#define __DEF__

#define CONNECT 0
#define DISCONNECT 1
#define GET_TIME 2
#define GET_NAME 3
#define LIST_CONNECTER 4
#define SEND 5
#define EXIT 6
#define SEND_BACK 7 //send要回包给发送客户端，用于区别发送客户端和接收客户端
#define MAX_CONN 5//max connection

// kbd ctrl
#define     ESC     27
#define     UP      183
#define     DOWN    184
#define     LEFT    186
#define     RIGHT   185
#define     ENTER   10
#define     KEYLEN  3

typedef struct package PACKAGE;
struct package{
    int type;
    int message_len;
    char buf[256];
};

const char* menu_head="+--------------------+\n|       menu         |\n+--------------------+";
#endif
