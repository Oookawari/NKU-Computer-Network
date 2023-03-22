/*协议控制*/
#ifndef PROTO_CTRL_H
#define PROTO_CTRL_H 1
#include "Protocol.h"
#include "cons.h"
#include <string>
#include <iostream>
#include <exception>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS
using namespace std;
/*控制单元 做啥事呢
分割字符串，将分割出的字符串送出，

*/
class Send_Action_Unit
{
public:
	/*发送用户名称
	*格式：SUN$type$(name_length)$nlen$(name)$end$
	*/
	static string Send_User_Name(string usrname) {
		/*构造发送串*/
		string msge = "";
		msge += SEND_USER_NAME;
		msge += "$type$";
		int UsrName_Len = usrname.length();
		msge += to_string(UsrName_Len);
		msge += "$nlen$";
		msge += usrname;
		msge += "$end$";
		return msge;
	}

	static string Sending(string str) {
		int token;
		token = str.find("<");
		string Act;
		string Rest;
		Act = str.substr(0, token);
		if (Act == I_GET) {
			return "RUL$type$$end$";
		}
		else if (Act == I_EXIT) {
			return "Exit";
		}
		else if (Act == I_SEND) {
			Rest = str.substr(token);
			string res = Send_To_User(Rest);
			if (res.length() <= MAX_MESS_LEN)
				return res;
			else
				return "OUT_OF_MAX_LENGTH";
		}
		else return "Invalid";

	}

	/*发送指定用户指定信息
	*格式：STU$type$(接收者id)$id$(时间)$time$(消息长度)$mslen$(发送消息)$end$
	*/
	static string Send_To_User(string str) {
		int token;
		token = str.find(">");
		string id_str = str.substr(1, token - 1);
		if(id_str == "*"){}
		else {
			int id;

			try
			{
				id = stoi(id_str);
			}
			catch (exception e)
			{
				cout << "输入id格式不合法！" << endl;
				return "Invalid";
			}
		}
		string Rest = str.substr(token + 1);
		if (Rest[0] != '<') {
			cout << "输入不合法！" << endl;
			return "Invalid";
		}
		if (Rest[Rest.length() - 1] != '>') {
			cout << "输入不合法！" << endl;
			return "Invalid";
		}
		string content = Rest.substr(1, Rest.length() - 2);
		/*构造发送串*/
		string msge = "";
		msge += SEND_TO_USER;
		msge += "$type$";
		msge += id_str;
		msge += "$id$";
		SYSTEMTIME sys_time;
		GetLocalTime(&sys_time);
		if (sys_time.wHour < 10) msge += "0";
		msge += to_string(sys_time.wHour);
		if (sys_time.wMinute < 10) msge += "0";
		msge += to_string(sys_time.wMinute);
		if (sys_time.wSecond < 10) msge += "0";
		msge += to_string(sys_time.wSecond);
		msge += "$time$";
		msge += to_string(content.length());
		msge += "$mslen$";
		msge += content;
		msge += "$end$";
		return msge;
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
	static void Get_Usr_Name_List(string msge) {
		int token;
		token = msge.find("$type$");
		string Act;
		string Rest;
		Act = msge.substr(0, token);
		Rest = msge.substr(token + 6);
		if (Act != SERVER_SEND_USER_LIST) {
			return;
		}
		token = Rest.find("$llen$");
		string list_len_str = Rest.substr(0, token);
		int list_len = stoi(list_len_str);
		Rest = Rest.substr(token + 6);
		cout << "----------------------" << endl;
		for (int i = 0; i < list_len; i++) {
			token = Rest.find("$nlen$");
			string name_len_str = Rest.substr(0, token);
			int name_len = stoi(name_len_str);
			Rest = Rest.substr(token + 6);
			string name = Rest.substr(0, name_len);
			Rest = Rest.substr(name_len);
			token = Rest.find("$id$");
			string id_str = Rest.substr(0, token);
			Rest = Rest.substr(token + 4);
			cout <<"用户id: " << id_str << " 用户名称 : " << name << endl;
		}
		cout << "-----------END-----------" << endl;
		return;
	}
	static void Get_Usr_Mess(string msge) {
		int token;
		token = msge.find("$type$");
		string Act;
		string Rest;
		Act = msge.substr(0, token);
		Rest = msge.substr(token + 6);
		if (Act != SERVER_DELIV_USER_MESS) {
			return;
		}
		token = Rest.find("$id$");
		string id_str = Rest.substr(0, token);
		int id = stoi(id_str);
		Rest = Rest.substr(token + 4);
		token = Rest.find("$time$");
		string time_h = Rest.substr(0, 2);
		string time_m = Rest.substr(2, 2);
		string time_s = Rest.substr(4, 2);
		Rest = Rest.substr(token + 6);
		token = Rest.find("$mslen$");
		string mess_len_str = Rest.substr(0, token);
		int mess_len = stoi(mess_len_str);
		Rest = Rest.substr(token + 7);
		string mess = Rest.substr(0, mess_len);
		cout << "----------------------" << endl;
		cout << "<接收消息>用户id: " << id << " " << "---" << time_h << ":" << time_m << ":" << time_s << endl;
		cout << mess << endl;
		cout << "----------------------" << endl;
		return;
	}
};

#endif // PROTO_CTRL_H
