#include <iostream>
#include <WinSock2.h>
#include "cons.h"
#include "debug_log.h"
#include "GBN.h"
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
	//接收端作为服务器
	SOCKET Server_Socket = Init_Socket();
	if (Server_Socket == NULL)
	{
		print_debug(DEBUG_ERROR, "Init Socket");
		return 0;
	}
	print_debug(DEBUG_INFO, "Init Socket Successfully");

	SOCKADDR_IN Client_Sockaddr;
	Client_Sockaddr.sin_family = SERVER_SOCKADDR_FAMILY;
	Client_Sockaddr.sin_port = htons(CLIENT_SOCKADDR_PORT);
	Client_Sockaddr.sin_addr.S_un.S_addr = inet_addr(CLIENT_SOCKADDR_ADDR);
	Runnable(Server_Socket, Client_Sockaddr);

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
	SOCKET Server_Socket = socket(SOCKET_AF, SOCKET_TYPE, SOCKET_PROTO);
	if (Server_Socket == INVALID_SOCKET) {
		string cont = "WSAStartup - ErrCode: " + to_string(Server_Socket);
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

	//Bind Socket
	sockaddr_in bind_sockaddr;
	bind_sockaddr.sin_family = SERVER_SOCKADDR_FAMILY;
	bind_sockaddr.sin_port = htons(SERVER_SOCKADDR_PORT);
	bind_sockaddr.sin_addr.S_un.S_addr = inet_addr(SERVER_SOCKADDR_ADDR);
	int bind_Err_Code = bind(Server_Socket, (SOCKADDR*)&bind_sockaddr, sizeof(bind_sockaddr));
	if (bind_Err_Code != 0) {
		string cont = "Bind - ErrCode: " + to_string(bind_Err_Code);
		print_debug(DEBUG_ERROR, cont);
		return NULL;
	}
	else {
		short port = ntohs(SERVER_SOCKADDR_PORT);
		string cont = "Bind Successfully - Port: " + to_string(port);
		print_debug(DEBUG_INFO, cont);
	}

	return Server_Socket;
}