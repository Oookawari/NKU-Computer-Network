/*常量*/
#ifndef CONS_H
#define CONS_H 1
#include <WinSock2.h>
#include <string>

using namespace std;
//port
const int CLI_PORT = 9999;
const int SER_PORT = 6666;
//最大发送接收串长度
const int MAX_MESS_LEN = 65536;
//Client.cpp: WSAStartup(wVersionRequested,)
const WORD wVersionRequested = MAKEWORD(2, 2);

//Client.cpp: socket(int af, int type, int protocol)
const int SOCKET_AF = AF_INET;
const int SOCKET_TYPE = SOCK_DGRAM;
const int SOCKET_PROTO = IPPROTO_UDP;

//Client.cpp: bind(SOCKET s, const sockaddr *addr, int namelen): sockaddr
const short CLIENT_SOCKADDR_FAMILY = AF_INET;

//Client.cpp: connect(SOCKET s, sockaddr* name, int name len): sockaddr

const short SERVER_SOCKADDR_FAMILY = AF_INET;
const u_short SERVER_SOCKADDR_PORT = htons(SER_PORT);
const u_short CLIENT_SOCKADDR_PORT = htons(CLI_PORT);
static const char* SERVER_SOCKADDR_ADDR = "127.0.0.1";
static const char* CLIENT_SOCKADDR_ADDR = "127.0.0.1";

const unsigned int MAX_SIZE = 20000000;

const string FILE_PATH = "1.jpg";

const int WINDOW_SIZE = 10;
//超时时间
const int Timeout = 100;
//校验位出错率
const int WRONG_CHECK_RATE = 1;
//丢包率
const int LOST_RATE = 1;
#endif // CONS_H

