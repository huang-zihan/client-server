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
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "def.h"

#define SERVER_PORT	4436 
#define BLOCK 0

extern int errno;

char buf[128];

struct student
{
	char name[32];
	int age;
};

int main(int argc, char *argv[])
{	
	int sockfd, connect_flag=0;
	struct sockaddr_in serverAddr;
	struct student stu;
	
	
	// if(argc != 3)
	// {
	// 	printf("usage: informLinuxClient serverIP type\n");
	// 	return -1;
	// }
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
		}
		//after connected
		else{
			PACKAGE package;
			printf("input function type:");
			scanf("%d", &package.type);
			package.message_len=0;

			switch(package.type){
				case DISCONNECT:
				case GET_TIME: break;
				default: break;
			}

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
			recv(sockfd, &package.type, sizeof(package.type), BLOCK);
			recv(sockfd, &package.message_len, sizeof(package.message_len), BLOCK);
			recv(sockfd, &package.buf, package.message_len, BLOCK);
			switch(package.type){
				case DISCONNECT:
					printf("%s",package.buf);
					connect_flag=0;
					break;
				case GET_TIME:
					int *pbuf=(int *)package.buf;
					printf("client: time:%d-%d-%d %d:%d:%d\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5));
					break;
			}
			// close(sockfd); //
		}
	}
	
	return 0;
}
