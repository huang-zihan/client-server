/*
 *	���������ҪĿ������˵��socket��̵Ļ������̣����Է�����/�ͻ��˵Ľ������̷ǳ��򵥣�
 *  ֻ���ɿͻ��������������һ��ѧ����Ϣ�Ľṹ��
 */

#include <stdio.h>
#include<stdlib.h>
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

#define SERVER_PORT	4436 // port
#define BLOCK 0

extern int errno;

struct info
{
	int sin_port;
	char sin_addr[20];
	int connfd;
};
typedef struct info info;
info clientlist[MAX_CONN];

void show_time(int);
void show_name(int);
void list_connecter(int);
void send_msg(int,PACKAGE*);

int main()
{	
	int sockfd, comfd;
	struct sockaddr_in serverAddr, clientAddr;
	int ret, iClientSize;
	void *ptr;
	// 
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket() failed! code:%d\n", errno);
		return -1;
	}

	// initialize
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset((void*)clientlist,0,MAX_CONN*sizeof(struct info));

	bzero(&(serverAddr.sin_zero), 8);
	if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		printf("bind() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}
		
	//
	if(listen(sockfd, MAX_CONN) == -1)
	{
		printf("listen() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}
		
	printf("Waiting for client connecting!\n");
	printf("tips : Ctrl+c to quit!\n");
	
	while(1){

		iClientSize = sizeof(struct sockaddr_in);
		if((comfd = accept(sockfd, (struct sockaddr *)&clientAddr,(socklen_t *) &iClientSize)) == -1)
		{
			continue;//循环调用accept（）直至返回有效句柄
		}
		printf("Accepted client: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		int i;
		for(i=0;i<MAX_CONN;i++)
		{
			if(clientlist[i].connfd==0)
			{
				break;
			}
		}
		clientlist[i].connfd=comfd;
		strcpy(clientlist[i].sin_addr,inet_ntoa(clientAddr.sin_addr));
		clientlist[i].sin_port=clientAddr.sin_port;
		if(fork()==0){
			// the child continue to connect server
			int connect_flag=1;
			send(comfd, "client: hello!\n", 16, BLOCK);
			PACKAGE package;
			memset((void*)&package,0,sizeof(PACKAGE));
			while(connect_flag){
				ret = recv(comfd, &package.type, sizeof(package.type), BLOCK);
				// printf("1\n");
				ret += recv(comfd, &package.message_len, sizeof(package.message_len), BLOCK);
				// printf("2\n%d",package.message_len);
				if(package.message_len>0){
					ret += recv(comfd, &package.buf, package.message_len,BLOCK);
					// printf("3\n");
				}
				if(ret != sizeof(package.type)+sizeof(package.message_len)+package.message_len){
					printf("recv() failed!\n");
					close(sockfd);
					close(comfd);
					return -1;
				}
				
				// printf("%d\n",package.type);
				if(package.type==DISCONNECT){
					package.message_len=19;
					memcpy(&package.buf,"client: good bye!\n",package.message_len);
					send(comfd, (char *)&package, sizeof(package), BLOCK);
					close(comfd);
					connect_flag=0;
					for(int i=0;i<MAX_CONN;i++)
					{
						if(clientlist[i].connfd==comfd)
						{
							memset((void*)&clientlist[i],0,sizeof(struct info));
							break;
						}
					}
				}

				int nLeft = package.message_len;
				while(nLeft > 0)
				{
					ret = recv(comfd, ptr, nLeft, 0);
					if(ret <= 0)
					{
						printf("recv() failed!\n");
						close(sockfd);
						close(comfd);
						return -1;
					}
					nLeft -= ret;
					ptr = (char *)ptr + ret;
				}
				// response
				switch (package.type){
					case GET_TIME:
						show_time(comfd);
						break;
					case GET_NAME:
						show_name(comfd);
						break;
					case LIST_CONNECTER:
						list_connecter(comfd);
						break;
					case SEND:
						send_msg(comfd,&package);
						break;
					case EXIT:
						if(connect_flag)
						{
							package.message_len=19;
							memcpy(&package.buf,"client: good bye!\n",package.message_len);
							send(comfd, (char *)&package, sizeof(package), BLOCK);
							close(comfd);
							for(int i=0;i<MAX_CONN;i++)
							{
								if(clientlist[i].connfd==comfd)
								{
									memset((void*)&clientlist[i],0,sizeof(struct info));
									break;
								}
							}
						}
						return 0;
					default:
						break;	
				}
			}
			if(!connect_flag){
				close(comfd);
				break;
			}
		}
	}
	printf("client %s:%d has been disconnected with process %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), getpid());
	wait(0);
	close(sockfd);
	return 0;
}

void show_time(int fd){
	time_t timer;
	struct tm* tblk;
	PACKAGE info;
	int *pbuf = (int *)info.buf;
	time(&timer);
	tblk = gmtime(&timer);
	info.type = GET_TIME;
	*(pbuf++) = tblk->tm_year+1900;
	*(pbuf++) = tblk->tm_mon;
	*(pbuf++) = tblk->tm_mday;
	*(pbuf++) = tblk->tm_hour;
	*(pbuf++) = tblk->tm_min;
	*(pbuf++) = tblk->tm_sec;
	pbuf-=6;
	// printf("client: time:%d year, %d day, %d:%d:%d\n", *(int*)info.buf, *((int*)info.buf+1), *((int*)info.buf+2), *((int*)info.buf+3), *((int*)info.buf+4));
	info.message_len=6*sizeof(int);
	send(fd,&info.type,sizeof(info.type),BLOCK);
	send(fd,&info.message_len,sizeof(info.message_len),BLOCK);
	send(fd,&info.buf,info.message_len,BLOCK);
}
void show_name(int fd)
{
	char buf[128];
	PACKAGE info;
	info.type=GET_NAME;
	gethostname(buf,sizeof(buf));
	strcpy(info.buf,buf);
	info.message_len=strlen(buf);
	send(fd,&info.type,sizeof(info.type),BLOCK);
	send(fd,&info.message_len,sizeof(info.message_len),BLOCK);
	send(fd,&info.buf,info.message_len,BLOCK);
}
void list_connecter(int fd)
{
	PACKAGE info;
	info.type=LIST_CONNECTER;
	char buf[256];
	char* ptr=buf;
	for(int i=0;i<MAX_CONN;i++)
	{
		if(clientlist[i].connfd)
		{
			*(int*)ptr=clientlist[i].connfd;
			ptr+=sizeof(int);
			*ptr=':';
			ptr++;
			strcpy(ptr,clientlist[i].sin_addr);//末尾出现'\0'
			ptr+=sizeof(clientlist[i].sin_addr);//指针移至'\0'处
			*ptr=':';//消除'\0'，防止strlen检测错误
			ptr++;
			*(int*)ptr=clientlist[i].sin_port;
			ptr+=sizeof(int);
			*ptr=' ';//两个客户端信息的间隔符
			ptr++;
		}
	}
	*ptr='\0';//结束标志
	strcpy(info.buf,buf);
	info.message_len=strlen(buf);
	send(fd,&info.type,sizeof(info.type),BLOCK);
	send(fd,&info.message_len,sizeof(info.message_len),BLOCK);
	send(fd,&info.buf,info.message_len,BLOCK);
}
void send_msg(int fd,PACKAGE* sent)
{
	PACKAGE info;
	info.type=SEND;
	char* str=sent->buf;
	char* ip,*port,*data;
	char* deli=":";
	ip=strsep(&str,deli);
	port=strsep(&str,deli);
	data=strsep(&str,deli);
	int i=0;
	int destination=0;
	for(;i<MAX_CONN;i++)
	{
		if(strcmp(ip,clientlist[i].sin_addr)==0 && atoi(port)==clientlist[i].sin_port)
		{
			destination=clientlist[i].connfd;
			break;
		}
	}
	if(i==MAX_CONN)
	{
		char tmp[]="client does not exist!";
		strcpy(info.buf,tmp);
		info.message_len=strlen(tmp);
		send(fd,(void*)&info,sizeof(PACKAGE),BLOCK);
		return;
	}
	else{
		char tmp[]="send success!";
		strcpy(info.buf,tmp);
		info.message_len=strlen(tmp);
	}
	PACKAGE infosend;
	infosend.type=SEND;
	strcpy(infosend.buf,data);
	infosend.message_len=strlen(data);
	if(send(destination,(void*)&infosend,sizeof(PACKAGE),BLOCK)<0)
	{
		char tmp[]="send fail!";
		strcpy(infosend.buf,tmp);
		infosend.message_len=strlen(tmp);
	}
	send(fd,(void*)&info,sizeof(PACKAGE),BLOCK);
}
