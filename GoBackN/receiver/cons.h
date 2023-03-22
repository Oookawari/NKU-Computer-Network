/*³£Á¿*/
#ifndef CONS_H
#define CONS_H 1
#include <WinSock2.h>
#include <string>
using namespace std;
//port
const int CLI_PORT = 9999;
const int SER_PORT = 6666;

//Server.cpp: WSAStartup(wVersionRequested,)
const WORD wVersionRequested = MAKEWORD(2, 2);

//Server.cpp: socket(int af, int type, int protocol)
const int SOCKET_AF = AF_INET;
const int SOCKET_TYPE = SOCK_DGRAM;
const int SOCKET_PROTO = IPPROTO_UDP;

//Server.cpp: bind(SOCKET s, const sockaddr *addr, int namelen): sockaddr
const short SERVER_SOCKADDR_FAMILY = AF_INET;

const u_short SERVER_SOCKADDR_PORT = htons(SER_PORT);
const u_short CLIENT_SOCKADDR_PORT = htons(CLI_PORT);
static const char* SERVER_SOCKADDR_ADDR = "127.0.0.1";
static const char* CLIENT_SOCKADDR_ADDR = "127.0.0.1";


const string FILE_PATH = "tttt.jpg";
#endif // CONS_H
