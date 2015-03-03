#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "my_recv.h"

#define INVALID_USERINFO	'n'
#define VALID_USERINFO		'y'

/*获取用户输入村到buf，buf的长度为len，用户输入数据以'\n'为结束标志*/

//输入用户名，然后通过fd发送出去
void input_userinfo(int conn_fd,const char *string)
{
	char input_buf[32];
	char recv_buf[BUFSIZE];
	int flag_userinfo;

	//输入用户信息直到正确为止
	do {
		printf("%s:",string);
		memset(input_buf,0,sizeof(input_buf));
		gets(input_buf);
	    if (send(conn_fd, input_buf, strlen(input_buf),0) < 0) {
		my_err("send",__LINE__);
		}

	//从连接套接字上读取一次数据
	if (recv(conn_fd,recv_buf,sizeof(recv_buf), 0) < 0) {
		printf("data is too long\n");
		exit(1);
	}

	if (recv_buf[0] == VALID_USERINFO) {
		flag_userinfo = VALID_USERINFO;
	} else {
		printf("%s error,input again",string);
		flag_userinfo = INVALID_USERINFO;
		}
	}while(flag_userinfo == INVALID_USERINFO);
}

int main(int argc,char **argv)
{
	int		ret;
	int 		conn_fd;
	int 		serv_port;
	struct sockaddr_in serv_addr;
	char 		recv_buf[BUFSIZE];
	char 		send_buf[BUFSIZE];
	pid_t 		pid;

	//初始化服务器端地址结构
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;

	//从命令行获取服务器端的端口与地址
	serv_port = atoi(argv[2]);
	serv_addr.sin_port = htons(serv_port);
	inet_aton(argv[1],&serv_addr.sin_addr);

	//创建一个TCP套接字
	conn_fd = socket(AF_INET,SOCK_STREAM,0);
	if (conn_fd < 0) {
		my_err("socket",__LINE__);
	}

	//向服务器端发送连接请求
	if(connect(conn_fd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
		my_err("connect",__LINE__);
	}
	printf("%d\n", conn_fd);

	//输入用户名和密码
	input_userinfo(conn_fd,"username");
	input_userinfo(conn_fd,"password");

	//打印欢迎消息
	memset(recv_buf, 0, sizeof(recv_buf));
	if((ret = recv(conn_fd,recv_buf,sizeof(recv_buf), 0)) < 0) {
		printf("data is too long in %d\n", __LINE__);
		exit(1);}
	
	recv_buf[ret-1]='\0';
	printf("%s\n",recv_buf);

	if((pid = fork()) == -1)
	{
		printf("error\n");
		return -1;
	}
	
	if(pid == 0)
	{	
		while(1) {
			memset(recv_buf, 0, sizeof(recv_buf));
			if((ret = recv(conn_fd,recv_buf,sizeof(recv_buf), 0)) <= 0) {
				printf("ret = %d\n",ret);
				printf("data is too long in %d\n", __LINE__);
				exit(1);
			}
			recv_buf[ret]='\0';
			printf(" %s\n", recv_buf);
			if(strcmp(recv_buf,"goodbye") == 0)
			{
				exit(0);
			}
		}	
	}
	else if (pid > 0)
	{
			printf("input message:\n");
		while(1)
		{
			memset(send_buf, 0, sizeof(send_buf));
			gets(send_buf);
		//	scanf("%s",send_buf);
			send(conn_fd,send_buf,sizeof(send_buf),0);
			if(strcmp(send_buf,"exit") == 0)
			{
				break;
			}
		}
	}
	close(conn_fd);
	return 0;
}
