/*协议控制*/
#ifndef PROTO_CTRL_H
#define PROTO_CTRL_H 1
#include "Protocol.h"
#include <string>
#include <pthread.h>
#include <WinSock2.h>
#include <vector>
#include "cons.h"
#include <windows.h>
using namespace std;

typedef struct {
	SOCKADDR_IN Client_Addr;
	SOCKET Client_Socket;
	pthread_t t_handle;
	int tid;
}Serv_Thread_Param;

class usr {
public:
	Serv_Thread_Param Sock_Inf;
	string Usr_Name;
	
	bool operator==(const usr& other)const
	{
		return Sock_Inf.tid == other.Sock_Inf.tid;
	}
};

class Recv_Action_Unit
{
public:
	static string Receive(string msge) {
		int token = msge.find("$type$");
		string Act = msge.substr(0, token);	
		return Act;
	}
	/*接收用户名称
	*格式：SUN$type$(name_length)$nlen$(name)$end$
	*/
	static string Get_Usr_Name(string msge) {
		int token;
		token = msge.find("$type$");
		string Act; 
		string Rest; 
		Act = msge.substr(0, token);
		Rest = msge.substr(token + 6);
		if (Act != CLIENT_SEND_USER_NAME) {
			return NULL;
		}
		token = Rest.find("$nlen$");
		string name_len_str = Rest.substr(0, token);
		int name_len = stoi(name_len_str);
		string user_name = Rest.substr(token + 6, name_len);
		return user_name;
	}
	
	/*发送指定用户指定信息
	*格式：STU$type$(接收者id)$id$(时间)$time$(消息长度)$mslen$(发送消息)$end$
	*/
	static int Send_To_User(string msge) {
		int token;
		token = msge.find("$type$");
		string Rest;
		Rest = msge.substr(token + 6);
		token = Rest.find("$id$");
		string id_str = Rest.substr(0, token);
		if (id_str == "*")
			return BROADCAST; 
		int id = stoi(id_str);
		return id;
	}
};

class Send_Action_Unit {
public:
	/*发送用户列表
	*格式：SUL$type$(list_length)$llen${(name_length)$nlen$(name)(id)$id$}$end$
	*/
	static string Send_User_List(vector<usr> list) {
		string msge;
		msge += SEND_USER_LIST;
		msge += "$type$";
		int List_Len = list.size();
		msge += to_string(List_Len);
		msge += "$llen$";
		for (int i = 0; i < List_Len; i++) {
			int UsrName_Len = list[i].Usr_Name.length();
			msge += to_string(UsrName_Len);
			msge += "$nlen$";
			msge += list[i].Usr_Name;
			msge += to_string(list[i].Sock_Inf.tid);
			msge += "$id$";
		}
		msge += "$end$";
		return msge;
	}

	/*转发用户信息
	*格式：DUM$type$(接收者id)$id$(时间)$time$(消息长度)$mslen$(发送消息)$end$
	*/
	static string Deliv_User_Mess(string msge, int from_id) {
		int token;
		token = msge.find("$type$");
		string Rest;
		Rest = msge.substr(token + 6);
		token = Rest.find("$id$");
		string rt_mess = "DUM$type$";
		rt_mess += to_string(from_id);
		rt_mess += Rest.substr(token);
		return rt_mess;
	}
};
#endif // PROTO_CTRL_H
