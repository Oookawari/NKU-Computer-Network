#include <iostream>
#include <string>
#include <stdio.h>
#include <WinSock2.h>
#include <pthread.h>
#include "Proto_Ctrl.h"
#include "cons.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")
#pragma warning(disable: 4996)
using namespace std;

string User_Name = "";
char* Local_IP;

/*��ȡ�û���*/
bool Init_Usr_Name() {
	cout << "Input your name: " << endl;
	string My_Name;
	if (cin >> My_Name) {
		User_Name = My_Name;
		return true;
	}
	else {
		return false;
	}
	
}

/*��ȡ����IP*/
bool Get_Local_Addr() {
	char buf[256] = "";
	struct hostent* ph = 0;
	gethostname(buf, 256);
	string hostName = buf;
	ph = gethostbyname(buf);
	Local_IP = inet_ntoa(*((struct in_addr*)ph->h_addr_list[0]));//�˴���ñ���IP
	printf("����IP��%s\n", Local_IP);
	return true;
}

/*��ʼ��Socket*/
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
	/*sockaddr_in Bind_Sockaddr;
	Bind_Sockaddr.sin_family = CLIENT_SOCKADDR_FAMILY;
	Bind_Sockaddr.sin_port = htons(CLIENT_SOCKADDR_PORT);
	Bind_Sockaddr.sin_addr.S_un.S_addr = inet_addr(Local_IP);

	int bind_err = SOCKET_ERROR; 
	int atpt = 0;
	
	��ʧ�ܣ����Ը����˿�
	while (bind_err == SOCKET_ERROR) {
		bind_err = bind(Client_Socket, (SOCKADDR*)&Bind_Sockaddr, sizeof(Bind_Sockaddr));
		if (bind_err == 0) {
			cout << "Success : Bind(Port: " << ntohs(CLIENT_SOCKADDR_PORT + atpt) << " )" << endl;
		}
		else {
			cout << "�󶨶˿�: " << ntohs(CLIENT_SOCKADDR_PORT + atpt) << "ʧ�ܣ����԰󶨶˿�" << ntohs(CLIENT_SOCKADDR_PORT + atpt + 1) << endl;
			atpt++;
		}
	}*/
	
	return Client_Socket;
}

/*���������������*/
bool Connect_Server(SOCKET Client_Socket) {
	SOCKADDR_IN Server_Sockaddr; 
	Server_Sockaddr.sin_family = SERVER_SOCKADDR_FAMILY;
	Server_Sockaddr.sin_port = htons(SERVER_SOCKADDR_PORT);
	Server_Sockaddr.sin_addr.S_un.S_addr = inet_addr(SERVER_SOCKADDR_ADDR);
	int conn_err = connect(Client_Socket, (SOCKADDR*)&Server_Sockaddr, sizeof(SOCKADDR));
	if (conn_err == 0) {
		return true;
	}
	else {
		return false;
	}
}


typedef struct {
	SOCKET Client_Socket;
	pthread_t t_handle;
}Client_Thread_Param;

/*����������mess*/
void* Listen_server(void* param) {
	Client_Thread_Param* p = (Client_Thread_Param*)param;
	char Recv_Buffer[MAX_MESS_LEN];
	while (1) {
		recv(p->Client_Socket, Recv_Buffer, MAX_MESS_LEN, 0);
		string act = Recv_Action_Unit::Receive(Recv_Buffer);
		if (act == SERVER_SEND_USER_LIST) {
			Recv_Action_Unit::Get_Usr_Name_List(Recv_Buffer);
		}
		else if (act == SERVER_DELIV_USER_MESS) {
			Recv_Action_Unit::Get_Usr_Mess(Recv_Buffer);
		}
	}
}

int main() {
	/*��ȡ�û���*/
	while (!Init_Usr_Name()) {
		cout << "Failed : Init User Name" << endl;
		return 0;
	}

	/*��ʼ��Socket*/
	SOCKET Client_Socket = Init_Socket();
	if (Client_Socket == NULL)
	{
		cout << "Failed : Init Socket" << endl;
		return 0;
	}

	/*���������������*/
	if (Connect_Server(Client_Socket)) {
		cout << "Success : Connect Server" << endl;
	}
	else {
		cout << "Failed : Connect Server" << endl;
		return 0;
	}

	/*�����û���*/
	string Send_Name = Send_Action_Unit::Send_User_Name(User_Name);
	send(Client_Socket, Send_Name.c_str(), Send_Name.length(), 0);

	/*�����û��б�*/
	char Recv_Buffer[MAX_MESS_LEN];
	char Send_Buffer[MAX_MESS_LEN];
	recv(Client_Socket, Recv_Buffer, MAX_MESS_LEN, 0);
	cout << "��ȡ�����û��б����£�" << endl;
	Recv_Action_Unit::Get_Usr_Name_List(Recv_Buffer);
	cout << "����: send<�û�id><����> ������ָ���û�������Ϣ " << endl;
	cout << "����: send<*><����> �����������û��㲥 " << endl;
	cout << "����: get ������ȡ�����û��б�" << endl;
	cout << "���: exit �����Ͽ����Ӳ��˳�Ӧ�ó���" << endl;
	cout << "----------------------" << endl;
	Client_Thread_Param t_param;
	t_param.Client_Socket = Client_Socket;
	//�����̼߳��������
	pthread_create(&(t_param.t_handle), NULL, Listen_server, (void*)&t_param);

	while (1) {
		string Next_Input;
		getline(cin, Next_Input);
		string Send_Mess = Send_Action_Unit::Sending(Next_Input);
		if (Send_Mess == "Exit") break;
		if (Send_Mess == "Invalid") {
			cout << "----------------------" << endl;
			continue;
		}
		if (Send_Mess == "OUT_OF_MAX_LENGTH") {
			cout << "���ͱ��ĳ��ȳ����������ֵ" << endl;
			cout << "----------------------" << endl;
		}
		send(Client_Socket, Send_Mess.c_str(), Send_Mess.length(), 0);
		cout << "----------------------" << endl;
	}
	cout << "����ر�" << endl;
	WSACleanup();
	return 0;
}