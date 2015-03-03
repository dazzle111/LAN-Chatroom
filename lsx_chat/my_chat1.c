#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "my_recv.h"

#define SERV_PORT	3333
#define MAX_BUF		128		//缓冲区长度
#define LISTENQ		8		//最长队列
#define USERNAME	0
#define	PASSWORD	1

char 		recv_buf[MAX_BUF];	//用于存储接收到的信息
char 		detail[MAX_BUF];	//用于存储发送的信息
struct sockaddr_in cli_addr,serv_addr;	//socket struct
int 		sock_fd;	//listen
int 		conn_fd;	//data transport
int 		ret;		//recv的返回值
int 		serv_port;		//port
int 		socket_num[20];			//用于记录登录用户socket
char		calculate_name[10][20]; 	//用于记录登录用户名字
int 		data_len = 0;			//在线人数
char 		mem_info[3][20];	//用来解析‘send’命令	



struct userinfo {  //保存用户名和密码的结构体
	char username[32];
	char password[32];
};

struct userinfo users[] = {
	{"1","1"},
	{"2","2"},
	{"clh","clh"},
	{"x1","x1"},
	{" "," "}     //以只含一个空格的字符串作为数组的结束标志
};

//查找用户名是否存在，存在返回该用户名的下标，不存在则返回-1.出错返回-2
int find_name(const char *name)
{
	int i;

	if (name == NULL) {
		printf("in find_name,NULL pointer");
		return -2;
	}
	for(i=0; users[i].username[0] != ' ';i++) {
		if (strcmp(users[i].username,name) == 0) {
			return i;
		}
	}

	return -1; 
}

int find_socket(int sock)	//寻找对应的套接字
{
	int i;
	for(i=0; i< data_len;i++)
	 	if(sock == socket_num[i])
			return i;

	return -1;

}

void send_data(int conn_fd,const char *string)
{
	if(send(conn_fd,string,strlen(string),0) < 0) {
		my_err("send",__LINE__);  //my_err函数在my_recv.h中声明
	}
}

void *my_run(int *arg)	//解析命令，发送接收发送信息。
{	
	int conn_fd = *arg;
	char tips[] = "send error";		//提示语句
	char goodbye[] = "goodbye"; 		//提示语句
	char tips1[30] = "name: ";		//修饰语句
	char tips2[] = " is gone~";		//修饰语句
	char tips3[100];			
	char tips4[100];
	char tips5[30];
	int j,i;
	int q,k=0;

	while(1){
		memset(recv_buf, 0, sizeof(recv_buf));
		if((ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0)) < 0) 
		{
			printf("error");
			return NULL;
		}
		if(*recv_buf == 0)
			continue;
		if((strcmp(recv_buf, "exit")) == 0)		//if recv_buf == exit,cancle the client
		{	
			if((q = find_socket(conn_fd)) < 0)
				printf("find sock failed\n");
			send(conn_fd,goodbye,sizeof(goodbye),0);
			for(i=0; i<data_len; i++)
				if(conn_fd == socket_num[i])
				{	
					k = i;
					i++;
				}
			strcat(tips1,calculate_name[k]);
			strcat(tips1,tips2);
			for(i=0; i<data_len; i++)
			{
				if(socket_num[i] == conn_fd)
					continue;
				send(socket_num[i],tips1,sizeof(tips1),0);
			}
			for(j=q; j<data_len-1; j++)
			{
				socket_num[j]=socket_num[j+1];
				strcpy(calculate_name[j],calculate_name[j+1]);		//zero the clients data
			}
			close(conn_fd);
			data_len--;
			return 0;
		}
		int t;
		if((strcmp(recv_buf,"ls")) == 0)	//如果接收到的信息为ls，返回当前在线用户名字与IP
		{
			//char aaa[10];
			//memset(aaa, 0, sizeof(aaa));
			memset(tips5,0,sizeof(tips5));
			for(i=0; i<data_len;i++)
			{
				strcpy(tips5,"Online : ");
				strcat(tips5,calculate_name[i]);
				sleep(1);
				send(conn_fd, tips5, strlen(tips5), 0);
			//	printf("calculate_name is :%s\n",calculate_name[i]);
				memset(tips5,0,sizeof(tips5));
			//	strcat(aaa, calculate_name[i]);
			}
				//send(conn_fd, calculate_name[i], strlen(calculate_name[i]), 0);
			//	send(conn_fd, aaa, strlen(calculate_name[i]), 0);
			continue;
		}
		
		
		if((strncmp(recv_buf,"send",4)) == 0)		//若字符串前四个字符为send,则将信息发送到指定用户
		{
			recv_buf[strlen(recv_buf)]='-';
			t=0;
			memset(mem_info, 0 , sizeof(mem_info));
			memset(tips3, 0, sizeof(tips3));
			for(i=0; i<3; i++) {
				j = 0;
				while(recv_buf[t] != '-')
				{
					mem_info[i][j++] = recv_buf[t];
					t++;
				}	
				t++;
			}
			if((q = find_name(mem_info[1])) < 0)
				send(conn_fd,tips,strlen(tips),0);
			printf("%d\n", socket_num[q]);
			for(i=0; i<data_len;i++)
			if(conn_fd == socket_num[i]){				//修饰语句
					strcat(tips3,calculate_name[i]);
					strcat(tips3," to you:");
					strcat(tips3,mem_info[2]);}
			if(( send(socket_num[q],tips3,sizeof(tips3),0)) <0)
				my_err("send",__LINE__);

		}
		else
			{	
				recv_buf[strlen(recv_buf) ]='\0';
				memset(tips4,0,sizeof(tips4));
				for(i=0;i<data_len;i++)
					if(conn_fd == socket_num[i])
					{						//修饰语句
						strcat(tips4,calculate_name[i]);
						strcat(tips4," to all:");
						strcat(tips4,recv_buf);
					}
						
				for(i=0;i<data_len;i++)
				{
					if(socket_num[i] == conn_fd)
						continue;
					send(socket_num[i],tips4,sizeof(tips4),0);
				}
			}
	}
}

int main(int argc, char *argv[])
{
	int 		optval; 		//option
	socklen_t	cli_len;		//socket len
	pthread_t	thid;			//用来接收信息
	int 		i = 0;			//used for "for"
	int 		flag_recv = USERNAME;	//用于验证用户姓名及密码
	int 		name_num;		//用于记录用户信息在数组中的位置

	//创建一个TCP套接字
	sock_fd = socket(AF_INET, SOCK_STREAM,0);
	if (sock_fd < 0) {
		my_err("socket",__LINE__);
	}

	//设置该套接字使之可以重新绑定端口
	optval = 1;
	if (setsockopt(sock_fd,SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval)) < 0) {
		my_err("setsockopt",__LINE__);
	}

	//初始化服务器端地址结构
	memset(&serv_addr,0,sizeof (struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);			//port
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);		//ip

	//将套接字绑定到本地端口
	if (bind(sock_fd,(struct sockaddr *)&serv_addr, sizeof (struct sockaddr_in)) < 0) 
	{
		my_err("bind",__LINE__);
	}

	//将套接字转化为监听套接字
	if (listen(sock_fd,LISTENQ) < 0) {
		my_err("listen",__LINE__);
	}

	cli_len = sizeof (struct sockaddr_in);
	while(1) {
	 	flag_recv = USERNAME;
		//通过accept接收客户端的连接请求，并返回连接套接字用收发数据
		conn_fd = accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_len);
		if (conn_fd < 0) {
			my_err("accept",__LINE__);
		}
		while(1) {
			if((ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0)) <0)
				my_err("recv",__LINE__);
			recv_buf[ret] = '\0'; //将数据结束标志'\n'替换城字符串结束标志

			if(flag_recv == USERNAME) {	//接收到的是用户名
				name_num = find_name(recv_buf);
				switch(name_num) {
					case -1:
						send_data(conn_fd,"n\n");
						break;
					case -2:
						exit(1);break;
					default:
						send_data(conn_fd,"y\n");
						flag_recv = PASSWORD;
						break;
				}

			} else if (flag_recv == PASSWORD) {  //接收到的是密码
				if (strcmp(users[name_num].password,recv_buf) == 0) {
					send_data(conn_fd,"y\n");
					sleep(1);
					send_data(conn_fd,"welcome!!\n");
					printf("%s login\n",users[name_num].username);
				}
				printf("accept a new client,ip:%s\n",inet_ntoa(cli_addr.sin_addr));
				socket_num[i] = conn_fd;
			//	printf("pre: %d %d\n", socket_num[1], socket_num[0]);
				strcpy(calculate_name[i], users[name_num].username);
				i++;
				data_len++;
				pthread_create(&thid, NULL, (void *)my_run, &conn_fd);	//调用线程处理接收的信息

				break;

			}			
		}
	}
	return 0;
}
