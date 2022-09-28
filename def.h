#ifndef __DEF__
#define __DEF__

#define CONNECT 0
#define DISCONNECT 1
#define GET_TIME 2
#define GET_NAME 3
#define LIST_CONNECTER 4
#define SEND 5
#define EXIT 6
#define RECV 7
#define SEND_BACK 8 //send要回包给发送客户端，用于区别发送客户端和接收客户端
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
#define SERVER_PORT	4436

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

const char* menu[8]={
	"CONNECT",
	"DISCONNECT",
	"GET_TIME",
	"GET_NAME",
	"LIST_CONNECTER",
	"SEND",
	"EXIT",
	"RECV"
};

const char* myserver="\033[33m  _____  ___.__.  ______  ____ _______ ___  __  ____ _______ \n /     \\<   |  | /  ___/_/ __ \\\\_  __ \\\\  \\/ /_/ __ \\\\_  __ \\\n|  Y Y  \\\\___  | \\___ \\ \\  ___/ |  | \\/ \\   / \\  ___/ |  | \\/\n|__|_|  // ____|/____  > \\___  >|__|     \\_/   \\___  >|__|   \n      \\/ \\/          \\/      \\/                    \\/\n\033[0m";
const char* myclient="\033[33m                        .__   .__                  __   \n  _____  ___.__.  ____  |  |  |__|  ____    ____ _/  |_ \n /     \\<   |  |_/ ___\\ |  |  |  |_/ __ \\  /    \\\\   __\\\n|  Y Y  \\\\___  |\\  \\___ |  |__|  |\\  ___/ |   |  \\|  |  \n|__|_|  // ____| \\___  >|____/|__| \\___  >|___|  /|__|  \n      \\/ \\/          \\/                \\/      \\/\n\033[0m";
const char* menu_head="+--------------------+\n|       menu         |\n+--------------------+";

#endif