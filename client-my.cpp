/*
 *	���������ҪĿ������˵��socket��̵Ļ������̣����Է�����/�ͻ��˵Ľ������̷ǳ��򵥣�
 *  ֻ���ɿͻ��������������һ��ѧ����Ϣ�Ľṹ��
 */
//informLinuxClient.cpp������Ϊ serverIP name age
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/wait.h>
#include <stddef.h>
#include <time.h>
#include "def.h"

// menu ctrl
#include <termios.h>
#include <unistd.h>
#include <iostream>

#include <pthread.h>


#define SERVER_PORT	4436 


using namespace std;

extern int errno;

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


char buf[128];


int main(int argc, char *argv[])
{	
	int sockfd, connect_flag=0;
	struct sockaddr_in serverAddr;
	
	while(1){
		int type;
		//connect to a server
		if(!connect_flag){
			char ip[20];
			do{
				int port=SERVER_PORT;
				printf("please input: 0(connect) server-ip port(%d) to connect first\n",SERVER_PORT);
				printf("or -1 to exit\n");
				scanf("%d", &type);
				if(type==-1){
					printf("bye!\n");
					exit(0);
				}
				scanf("%s %d", ip, &port);
			}while(type!=0);

			if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{
				printf("socket() failed! code:%d\n", errno);
				return -1;
			}

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(SERVER_PORT);
			serverAddr.sin_addr.s_addr = inet_addr(ip);
			bzero(&(serverAddr.sin_zero), 8);

			printf("connecting!\n");
			if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
			{
				printf("connect() failed! code:%d\n", errno);
				close(sockfd);
				return -1;
			}
			printf("Connected!\n");
			connect_flag=1;

			// receive the welcome message
			recv(sockfd, buf, 17, BLOCK);
			printf("%s",buf);
			getchar();
		}
		//after connected
		else{
			PACKAGE package;
			package.message_len=0;
			//******
			// set terminal
			static struct termios oldt, newt;
			char ctrl_key[KEYLEN];
			int user_choice=0;
			tcgetattr( STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~(ICANON);         // c_lflag 本地模式
			tcsetattr( STDIN_FILENO, TCSANOW, &newt);
			system("stty -echo");
			int sum;
			printf("%s\n",menu_head);
			do{
				printf("%d		%s",user_choice, menu[user_choice]);
				fflush(stdout);
				read(0, &ctrl_key, 3);
				sum=ctrl_key[0]+ctrl_key[1]+ctrl_key[2];
				if(sum==UP){
					user_choice=(user_choice-1+8)%8;
				}else if(sum==DOWN){
					user_choice=(user_choice+1)%8;
				}
				memset(ctrl_key,0,KEYLEN);
				printf("\r                              \r");
				fflush(stdout);
			}while(sum!=ENTER);
			puts("");		
			system("stty echo");
			tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
			//******
			package.type=user_choice;
			printf("%d\n",package.type);
			switch(package.type){
				case DISCONNECT:
				case GET_NAME:
				case LIST_CONNECTER:
				case EXIT:
				case GET_TIME:
					strcpy(package.buf,"\0");//包的内容设置为空
					package.message_len=0; 
					break;
				case SEND:
					printf("chatting mode:");
					scanf("%s",package.buf);
					package.message_len=strlen(package.buf);
					break;
				default: break;
			}

			if(package.type==RECV){
				printf("recv called!\n");
				// while(recv(sockfd, &package.type, sizeof(package.type), NONEBLOCK)>=0){
					memset(&package.buf,0,sizeof(package.buf));
					package.message_len=0;
					recv(sockfd, &package.type, sizeof(package.type), BLOCK);
					recv(sockfd, &package.message_len, sizeof(package.message_len), BLOCK);
					if(package.message_len>0){
						recv(sockfd, &package.buf, package.message_len, BLOCK);
					}
					if(package.message_len>0){
						printf("recv: %s\n",package.buf);
					}
				// }
				continue; //no send message step, so break.
			}
			
			// printf("%d %d %s\n",package.type,package.message_len,package.buf);
			if(send(sockfd, &package.type, sizeof(package.type), BLOCK) == -1 
			or send(sockfd, &package.message_len, sizeof(package.message_len), BLOCK) == -1 
			or send(sockfd, &package.buf, package.message_len, BLOCK) == -1
			){
				printf("send() failed!\n");
				close(sockfd);
				return -1;
			}

			switch(package.type){
				case DISCONNECT:
				case GET_TIME: break;
				default: break;
			}
			memset(&package.buf,0,sizeof(package.buf));
			recv(sockfd, &package.type, sizeof(package.type), BLOCK);
			recv(sockfd, &package.message_len, sizeof(package.message_len), BLOCK);
			recv(sockfd, &package.buf, package.message_len, BLOCK);
			// printf("message: %s\n",package.buf);
			// printf("%ld\n",strlen(package.buf));
			switch(package.type){
				case DISCONNECT:
					printf("%s",package.buf);
					connect_flag=0;
					break;
				case GET_TIME:{
					int *pbuf=(int *)package.buf;
					printf("client: time:%d-%d-%d %d:%d:%d\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5));
					break;
				}
				case GET_NAME:
					printf("client : name:%s\n",package.buf);
					break;
				case LIST_CONNECTER:{
					if(package.message_len==0) break;
					char* tmp=package.buf;

					int num = *(int*)tmp;
					tmp+=sizeof(int);
					printf("num:%d\n",num);

					for(int i=0;i<num;i++){
						int connfd=*(int*)tmp;
						tmp+=sizeof(int);
						char sin_addr[20];
						int cnt=0;
						while(*(tmp++)!='\0'){
							sin_addr[cnt++]=*tmp;
						}

						int port=*(int*)tmp;
						tmp+=sizeof(int);
						printf("client : connfd:%d sin_addr:%s port:%d\n",connfd,sin_addr,port);
					}
					
					break;
				}
				case SEND_BACK:
					printf("client: send status:%s\n",package.buf);
					break;
				case SEND:
					printf("len: %ld\n", strlen(package.buf));
					printf("client: receive data:%s\n",package.buf);
					break;
				case EXIT:
					printf("%s",package.buf);
					exit(0);
					break;
			}
			// close(sockfd); //
		}
	}
	
	return 0;
}


// void* receiving_message(void *arg){
// 	thread_arg *pargs = (thread_arg *)arg;
// 	int sockfd = pargs->sockfd;
// 	PACKAGE recv_pac;
// 	while (1){
// 		recv(sockfd, &recv_pac.type, sizeof(recv_pac.type), BLOCK);
// 		recv(sockfd, &recv_pac.message_len, sizeof(recv_pac.message_len), BLOCK);
// 		recv(sockfd, &recv_pac.buf, recv_pac.message_len, BLOCK);
// 		printf("receive: %s\n",recv_pac.buf);
// 	}
	
// }