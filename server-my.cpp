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

#include <pthread.h>

#define BLOCK 0

extern int errno;


static info clientlist[MAX_CONN];

void show_time(int);
void show_name(int);
void list_connecter(int);
void send_msg(int,PACKAGE*);
void* thread_work(void *arg);
int main(){	

	int sockfd, comfd;
	struct sockaddr_in serverAddr, clientAddr;
	int iClientSize;
	thread_arg new_arg;
	system("clear");
	printf("%s\n\n\n",myserver);
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

		pthread_t tid;
		new_arg.comfd=comfd;
		new_arg.sockfd=sockfd;
		new_arg.clientAddr=clientAddr;
		pthread_create(&tid, NULL, thread_work, &new_arg);
	}
	close(sockfd);
	return 0;
}

void show_time(int fd){
	time_t timer;
	struct tm* tblk;
	PACKAGE info;
	int *pbuf = (int *)info.buf;
	time(&timer);
	tblk = localtime(&timer);
	info.type = GET_TIME;
	*(pbuf++) = tblk->tm_year+1900;
	*(pbuf++) = tblk->tm_mon + 1;
	*(pbuf++) = tblk->tm_mday;
	*(pbuf++) = tblk->tm_hour;
	*(pbuf++) = tblk->tm_min;
	*(pbuf++) = tblk->tm_sec;
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
	char* ptr=info.buf;
	int connect_num=0;
	int message_len=0;
	
	for(int i=0;i<MAX_CONN;i++)
		if(clientlist[i].connfd) connect_num++;
	*(int*)ptr=connect_num;
	ptr+=sizeof(int);
	message_len+=sizeof(int);

	for(int i=0;i<MAX_CONN;i++)
	{
		if(clientlist[i].connfd)
		{
			*(int*)ptr=clientlist[i].connfd;
			ptr+=sizeof(int);
			message_len+=sizeof(int);
			*ptr=':';
			ptr++;
			message_len++;
			strcpy(ptr,clientlist[i].sin_addr);
			ptr+=strlen(clientlist[i].sin_addr) + 1;
			message_len += strlen(clientlist[i].sin_addr) + 1;
			*(int*)ptr=clientlist[i].sin_port;
			ptr+=sizeof(int);
			message_len+=sizeof(int);
		}
	}
	info.message_len=message_len;

	send(fd,&info.type,sizeof(info.type),BLOCK);
	send(fd,&info.message_len,sizeof(info.message_len),BLOCK);
	send(fd,&info.buf,info.message_len,BLOCK);
}
void send_msg(int fd,PACKAGE* sent)
{
	PACKAGE info;
	info.type=SEND;
	printf("data=%s\n",sent->buf);
	char* str=sent->buf;
	char* ip,*port,*data;
	const char* deli=":";
	ip=strsep(&str,deli);
	port=strsep(&str,deli);
	data=strsep(&str,deli);
	printf("ip=%s port=%s data=%s",ip,port,data);
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
		if(send(fd,&info.type,sizeof(info.type),BLOCK)<0
		|| send(fd,&info.message_len,sizeof(info.message_len),BLOCK)<0
		|| send(fd,&info.buf,info.message_len,BLOCK)<0){
			printf("send fail\n");
		}
		return;
	}

	PACKAGE infosend;
	infosend.type=SEND;
	strcpy(infosend.buf,data);
	infosend.message_len=strlen(data);
	infosend.buf[infosend.message_len]=0;

	PACKAGE ret_package;
	ret_package.type=SEND;
	if(send(destination,&infosend.type,sizeof(infosend.type),BLOCK)<0 ||
	send(destination,&infosend.message_len,sizeof(infosend.message_len),BLOCK)<0 ||
	send(destination,&infosend.buf,infosend.message_len,BLOCK)<0)
	{
		printf("send fail\n");
		char tmp[]="send fail!";
		strcpy(ret_package.buf,tmp);
		ret_package.message_len=strlen(tmp);
	}
	else{
		printf("send success\n");
		char tmp[]="send success!";
		strcpy(ret_package.buf,tmp);
		ret_package.message_len=strlen(tmp);
	}
	printf("retpackage type=%d\n",ret_package.type);
	if(send(fd,&ret_package.type,sizeof(ret_package.type),BLOCK)<0 ||
	send(fd,&ret_package.message_len,sizeof(ret_package.message_len),BLOCK)<0 ||
	send(fd,&ret_package.buf,ret_package.message_len,BLOCK)<0)
	{
		printf("send back fail\n");
	}
}

void* thread_work(void *arg){
	// the new thread continue to connect server
	int ret;
	void *ptr;
	int connect_flag=1;
	thread_arg* pthreadargs=(thread_arg*)arg;
	int comfd=pthreadargs->comfd, sockfd=pthreadargs->sockfd;
	struct sockaddr_in clientAddr = pthreadargs->clientAddr;
	int ret_value=0;
	send(comfd, "client: hello!\n", 16, BLOCK);
	PACKAGE package;
	while(connect_flag){
		memset((void*)&package,0,sizeof(PACKAGE));
		ret = recv(comfd, &package.type, sizeof(package.type), BLOCK);
		ret += recv(comfd, &package.message_len, sizeof(package.message_len), BLOCK);
		if(package.message_len>0){
			ret += recv(comfd, &package.buf, package.message_len,BLOCK);
		}
		if(ret != sizeof(package.type)+sizeof(package.message_len)+package.message_len){
			printf("recv() failed!\n");
			close(sockfd);
			close(comfd);
			ret_value=-1;
			return NULL;
		}
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
					break; //break for
				}
			}
			break;// break while loop as disconnect
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
				return NULL;
			default:
				break;
		}
	}
	printf("client %s:%d has been disconnected with process %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), getpid());
	close(comfd);
	return NULL;
}
