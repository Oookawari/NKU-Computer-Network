#include <iostream>
#include <string>
#include <stdio.h>
#include <WinSock2.h>
#include "cons.h"
#include "Proto_Ctrl.h"
#include <pthread.h>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")
#pragma warning(disable: 4996)
using namespace std;

char* Local_IP;
	
/*获取本机IP*/
bool Get_Local_Addr() {
	char buf[256] = "";
	struct hostent* ph = 0;
	gethostname(buf, 256);
	string hostName = buf;
	ph = gethostbyname(buf);
	Local_IP = inet_ntoa(*((struct in_addr*)ph->h_addr_list[0]));//此处获得本机IP
	printf("本机IP：%s\n", Local_IP);
	return true;
}

/*初始化Socket*/
SOCKET Init_Socket() {

	WSADATA wsaData;
	int Stp_Err_Code = WSAStartup(wVersionRequested, &wsaData);
	if (Stp_Err_Code) {
		cout << "Failed : WSAStartup - " << Stp_Err_Code << endl;
		return NULL;
	}

	SOCKET Client_Socket = socket(SOCKET_AF, SOCKET_TYPE, SOCKET_PROTO);
	if (Client_Socket == INVALID_SOCKET) {
		cout << "Failed : invalid_Socket" << endl;
		return NULL;
	}
	Get_Local_Addr();

	sockaddr_in bind_sockaddr;
	bind_sockaddr.sin_family = SERVER_SOCKADDR_FAMILY;
	bind_sockaddr.sin_port = htons(SERVER_SOCKADDR_PORT);
	bind_sockaddr.sin_addr.S_un.S_addr = inet_addr(SERVER_SOCKADDR_ADDR);

	int bind_err = bind(Client_Socket, (SOCKADDR*)&bind_sockaddr, sizeof(bind_sockaddr));
	if (bind_err == 0) {
		cout << "Success : Bind(Port: " << ntohs(SERVER_SOCKADDR_PORT) << " )" << endl;
	}
	else {
		cout << "Failed : bind error" << endl;
		return NULL;
	}
	return Client_Socket;
}

vector<usr> User_List;

/*简单实现vector查找函数（查找是否存在）*/
bool vector_find(vector<unsigned int> v, unsigned int element) {
	vector<unsigned int>::iterator it;
	it = find(v.begin(), v.end(), element);
	if (it != v.end())
		return true;
	else
		return false;
}

void* Serv(void* param) {
	Serv_Thread_Param* p = (Serv_Thread_Param*)param;
	cout << "<NEWCONNECTION> running: " << p->tid << "addr - " << inet_ntoa(p->Client_Addr.sin_addr) << ", port - " << ntohs(p->Client_Addr.sin_port) << endl;
	usr User;
	User.Sock_Inf = *p;
	char Recv_Buffer[MAX_MESS_LEN];
	string Send_Buffer;
	/*接收用户返回用户名*/
	recv(User.Sock_Inf.Client_Socket, Recv_Buffer, MAX_MESS_LEN, 0);
	User.Usr_Name = Recv_Action_Unit::Get_Usr_Name(Recv_Buffer);
	User_List.push_back(User);
	cout << "<GETNAME> addr - " << inet_ntoa(p->Client_Addr.sin_addr) << ", port - " << ntohs(p->Client_Addr.sin_port) << ", name : " << User.Usr_Name << endl;
	Send_Buffer = Send_Action_Unit::Send_User_List(User_List);
	send(User.Sock_Inf.Client_Socket, Send_Buffer.c_str(), Send_Buffer.length(), 0);
	while (1) {
		int rtv = recv(User.Sock_Inf.Client_Socket, Recv_Buffer, MAX_MESS_LEN, 0);
		if (rtv == -1) break;
		string act = Recv_Action_Unit::Receive(Recv_Buffer);

		if (act == CLIENT_SEND_TO_USER) {
			int Send_To_Id = Recv_Action_Unit::Send_To_User(Recv_Buffer);
			if (Send_To_Id == -1) {
				Send_Buffer = Send_Action_Unit::Deliv_User_Mess(Recv_Buffer, User.Sock_Inf.tid);

				cout << "<BROADCAST>" << " from: " << User.Sock_Inf.tid << endl;

				for (int i = 0; i < User_List.size(); i++) {
					send(User_List[i].Sock_Inf.Client_Socket, Send_Buffer.c_str(), Send_Buffer.length(), 0);
				}
			}
			else {
				usr send_to;
				for (int i = 0; i < User_List.size(); i++) {
					if (User_List[i].Sock_Inf.tid == Send_To_Id) {
						send_to = User_List[i];
						break;
					}
				}
				Send_Buffer = Send_Action_Unit::Deliv_User_Mess(Recv_Buffer, User.Sock_Inf.tid);

				cout << "<SENDLIST>" << " from: " << User.Sock_Inf.tid << " to:" << send_to.Sock_Inf.tid << endl;
				send(send_to.Sock_Inf.Client_Socket, Send_Buffer.c_str(), Send_Buffer.length(), 0);
			}
		}
		else if (act == CLIENT_REQUEST_USER_LIST) {
			Send_Buffer = Send_Action_Unit::Send_User_List(User_List);
			send(User.Sock_Inf.Client_Socket, Send_Buffer.c_str(), Send_Buffer.length(), 0);
		}
	}
	cout << "<EXIT> exit: " << p->tid << "addr - " << inet_ntoa(p->Client_Addr.sin_addr) << ", port - " << ntohs(p->Client_Addr.sin_port) << endl;
	for (auto it = User_List.begin(); it != User_List.end(); it++) {
		if (*it == User) {
			User_List.erase(it);
			break;
		}
	}
	return NULL;
}

int main() {
	/*初始化Socket*/
	SOCKET Server_Socket = Init_Socket();
	if (Server_Socket == NULL)
	{
		cout << "Failed : Init Socket" << endl;
		return 0;
	}
	cout << "Succeed : Init Socket" << endl;
	/*监听客户端连接*/ 
	if (listen(Server_Socket, 5) == INVALID_SOCKET) {
		cout << "Failed : Listen Socket: INVALID_SOCKET" << endl;
		return 0;
	}
	cout << "Succeed : Listen Socket" << endl;

	/*创建用户列表*/
	while (true) {
		SOCKADDR_IN Client_Addr;
		int addrLen = sizeof(SOCKADDR_IN);
		SOCKET Client_Socket = accept(Server_Socket, (SOCKADDR*)&Client_Addr, &addrLen);
		if (Client_Socket == INVALID_SOCKET) {
			cout << "Failed : Accept Socket: INVALID_SOCKET" << endl;
			continue;
		}
		Serv_Thread_Param t_param;
		t_param.Client_Addr = Client_Addr;
		t_param.Client_Socket = Client_Socket;
		t_param.tid = User_List.size();
		//创建线程服务连接的客户端
		pthread_create(&(t_param.t_handle), NULL, Serv, (void*)&t_param);
	}
	WSACleanup();
	return 0;
}