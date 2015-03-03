# LAN-Chatroom
socket实现的局域网聊天室
局域网聊天室设计书
一、项目总述：

利用暑假所学的文件，进程，线程等相关知识，简单设计一个局域网聊天室。

二、基本功能：

1、 支持更新好友在线列表。
2、 支持私聊功能。
3、 支持群聊功能。

三、模块介绍：
1、辅助函数：
		my_err(char *err_string,int line)                       //错误处理函数

2、服务端：
		int find_name(const chat *name)		                      //查找用户名
		int find_socket(int sock)		                         		//查找相应的套接字
		void send_data(int conn_fd,char *string)								//发送内容
		void *my_run(int *arg)		                          		//线程函数，用于解析客户端的各种信息
    int main(int argc,char **argv)                    			//接收客户端的连接，创建线程处理用户信息

4、 客户端：
void input_userinfo(int conn_fd,char *string)       				//登陆验证
int main(int argc,char **argv)	                         		//处理客户端的接发信息

四、函数说明：
①.服务端：
	int main(int argc,char **argv)
		主函数，一直处于循环accept状态。当客户端与服务端进行连接并登陆验证完毕之后，
	将创建线程进行解析处理该客户端发送给服务端的信息。

	int find_name(const char *name)
		 从记录客户端姓名的列表中寻找name，若找到则返回该名字在对应的列表中的序号，
  之后主要用于在记录客户端套接字列表中找到对应姓名的套接字，然后进行传输内容。
  若未找到相应的套接字则返回-1；

  int find_socket(int sock)
	    寻找客户端套接字sock在套接字列表中的序号，若找到则返回该套接字在对应列表中的序号，
	之后主要用于在客户端姓名列表中找到对应的姓名，然后对传输的内容进行标识，
	提示客户端信息的来源。若未找到对应的姓名，返回-1.

  void *my_run(int *arg) 
	    线程函数，通过参数*arg获取客户端对应的套接字。用局部变量记录该套接字，
	然后通过该套接字解析对应客户所发送的信息，并进行转发或退出操作。
	例如：”ls”命令:将记录用户登录姓名的列表发送至查看好友在线信息的客户端上，
	以便用户了解自己的上线好友姓名；”send”命令:通过判断客户端发送的信息的前四个字符是否未”send”，
	若是，则截取后面的信息，找到对应的用户名及套接字进行转发，若没有”send”字符的信息默认为群聊处理。
	”send”命令的格式为:send-用户姓名-内容(中间用”-”隔开)；”exit”命令:
	若接受到用户发送的信息为”exit”,则返回信息”goodbye”给用户，欢迎用户下次再登陆，然后断开连接。

②客户端:
  void input_userinfo(int conn_fd,char *string)
		  通过套接字将用户姓名与密码发送至服务器验证，若服务器验证成功返回y,则可以正常用服务器进行通信，
	否则将会一直停留在验证信息的阶段，可用ctrl+c退出.

	int main(int argc,char **argv)
		主函数，通过命令行直接获取参数。例如argv[1]为ip地址，argv[2]为端口号。若输入错误则会提示段错误。
	之所以如此设计，是为了方便调试，可以提高查找在编写过程产生的bug的速度。
	当获取命令行参数正确之后，用服务端进行连接，
	然后调用void input_userinfo(int conn_fd,char *string)验证用户登录信息，
	然后创建子进程负责接收服务端发送的信息，父进程负责发送信息至服务端。
