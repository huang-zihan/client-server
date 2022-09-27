#ifndef __DEF__
#define __DEF__

#define CONNECT 0
#define DISCONNECT 1
#define GET_TIME 2
#define GET_NAME 3
#define LIST_CONNECTER 4
#define SEND 5
#define EXIT 6
#define SEND_BACK 8 //send要回包给发送客户端，用于区别发送客户端和接收客户端
#define RECV 7
#define MAX_CONN 5 //max connection

// kbd ctrl
#define     ESC     27
#define     UP      183
#define     DOWN    184
#define     LEFT    186
#define     RIGHT   185
#define     ENTER   10
#define     KEYLEN  3

#define BLOCK 0
#define NONEBLOCK 1
#define SERVER_PORT	4434

typedef struct package PACKAGE;
struct package{
    int type;
    int message_len;
    char buf[256];
};

typedef struct info info;
struct info{
	int sin_port;
	char sin_addr[20];
	int connfd;
};

typedef struct thread_arg thread_arg;
struct thread_arg{
	int comfd;
	int sockfd;
	struct sockaddr_in clientAddr;
};

const char* menu_head="+--------------------+\n|       menu         |\n+--------------------+";
#endif
