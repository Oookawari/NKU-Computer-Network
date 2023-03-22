#include <iostream>
#include <WinSock2.h>
#include "cons.h"
#include "debug_log.h"
#include "NewReno.h"
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
using namespace std;

/*储存本机ip*/
char* Local_IP;

/*获取本机IP*/
bool Get_Local_Addr();

/*初始化Socket*/
SOCKET Init_Socket();


int main() {
	//发送端作为客户端
	SOCKET Client_Socket = Init_Socket();
	if (Client_Socket == NULL)
	{
		print_debug(DEBUG_ERROR, "Init Socket");
		return 0;
	}
	print_debug(DEBUG_INFO, "Init Socket Successfully");

	SOCKADDR_IN Server_Sockaddr;
	Server_Sockaddr.sin_family = SERVER_SOCKADDR_FAMILY;
	Server_Sockaddr.sin_port = htons(SERVER_SOCKADDR_PORT);
	Server_Sockaddr.sin_addr.S_un.S_addr = inet_addr(SERVER_SOCKADDR_ADDR);

	/*设置recvfrom超时时间*/
	struct timeval tm;
	tm.tv_sec = Timeout;   //毫秒
	tm.tv_usec = 0;  //微秒
	setsockopt(Client_Socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tm, sizeof(tm));

	Thread_Param t_param;
	t_param.t_handle;
	mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_rwlock_init(&state_lock, NULL);
	pthread_rwlock_init(&send_lock, NULL);
	pthread_create(&(t_param.t_handle), NULL, Recv_Thread, (void*)&t_param);
	Send_Thread(Client_Socket, Server_Sockaddr);
	return 0;
}

bool Get_Local_Addr() {
	char buf[256] = "";
	struct hostent* ph = 0;
	gethostname(buf, 256);
	string hostName = buf;
	ph = gethostbyname(buf);
	Local_IP = inet_ntoa(*((struct in_addr*)ph->h_addr_list[0]));//此处获得本机IP
	return true;
}

SOCKET Init_Socket() {
	//Startup
	WSADATA wsaData;
	int Stp_Err_Code = WSAStartup(wVersionRequested, &wsaData);
	if (Stp_Err_Code) {
		string cont = "WSAStartup - ErrCode: " + to_string(Stp_Err_Code);
		print_debug(DEBUG_ERROR, cont);
		return NULL;
	}
	print_debug(DEBUG_INFO, "WSAStartup Successfully");

	//Socket
	SOCKET Client_Socket = socket(SOCKET_AF, SOCKET_TYPE, SOCKET_PROTO);
	if (Client_Socket == INVALID_SOCKET) {
		string cont = "WSAStartup - ErrCode: " + to_string(Client_Socket);
		print_debug(DEBUG_ERROR, cont);
		return NULL;
	}
	print_debug(DEBUG_INFO, "SocketCreate Successfully");

	//Getaddr
	bool Get_Err_Code = Get_Local_Addr();
	if (Get_Err_Code) {
		string myIp = Local_IP;
		print_debug(DEBUG_INFO, "GetAddr Successfully - IP: " + myIp);
	}

	sockaddr_in bind_sockaddr;
	bind_sockaddr.sin_family = CLIENT_SOCKADDR_FAMILY;
	bind_sockaddr.sin_port = htons(CLIENT_SOCKADDR_PORT);
	bind_sockaddr.sin_addr.S_un.S_addr = inet_addr(CLIENT_SOCKADDR_ADDR);
	int bind_Err_Code = bind(Client_Socket, (SOCKADDR*)&bind_sockaddr, sizeof(bind_sockaddr));
	if (bind_Err_Code != 0) {
		string cont = "Bind - ErrCode: " + to_string(bind_Err_Code);
		print_debug(DEBUG_ERROR, cont);
		return NULL;
	}
	else {
		short port = ntohs(CLIENT_SOCKADDR_PORT);
		string cont = "Bind Successfully - Port: " + to_string(port);
		print_debug(DEBUG_INFO, cont);
	}

	return Client_Socket;
}