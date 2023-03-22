/*常量*/
#ifndef CONS_H
#define CONS_H 1
#include <WinSock2.h>
#include <string>
//port
const int SER_PORT = 6666;

//最大发送接收串长度
const int MAX_MESS_LEN = 500;

//Server.cpp: WSAStartup(wVersionRequested,)
const WORD wVersionRequested = MAKEWORD(2, 2);

//Server.cpp: socket(int af, int type, int protocol)
const int SOCKET_AF = AF_INET;
const int SOCKET_TYPE = SOCK_STREAM;
const int SOCKET_PROTO = IPPROTO_TCP;

//Server.cpp: bind(SOCKET s, const sockaddr *addr, int namelen): sockaddr
const short SERVER_SOCKADDR_FAMILY = AF_INET;
const u_short SERVER_SOCKADDR_PORT = htons(SER_PORT);
const char* SERVER_SOCKADDR_ADDR = "192.168.204.1";

const int BROADCAST = -1;
#endif // CONS_H
