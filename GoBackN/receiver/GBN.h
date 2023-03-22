#ifndef GBN_H
#define GBN_H 1
#include <WinSock2.h>
#include <string>

#define check_cr(sum) {if(sum >= 0x100) sum -= 0xFF;}
using namespace std;
/*statr list*/
const unsigned int INIT = 0;
const unsigned int SYN_RECV = 1;
const unsigned int ESTABLISHED = 2;
const unsigned int CLOSE_WAIT = 3;
const unsigned int CLOSED = 4;
const unsigned int EXIT = 5;
const unsigned int RECVING = 6;

/*flags*/
const unsigned int ACK = (1 << 7);
const unsigned int SYN = (1 << 6);
const unsigned int FIN = (1 << 5);
const unsigned int SEN = (1 << 4);
const unsigned int NAK = (1 << 2);
const unsigned int FINAL = (1 << 1);

const unsigned int MAX_LENGTH = 65535;
static char Recv_Buffer[MAX_LENGTH];
void Runnable(SOCKET server_socket, SOCKADDR_IN client_addr);

/*协议构成：
文件头：
长度 16位(data+16)
校验和 8位
预留位 8位 （ACK|SYN|FIN|RECV|0|NAK|FINAL|0）
ack 16位
data
*/

/*产生校验和 8位*/
short generate_checksum(string message);
bool check_checksum(string message);
string send(short flags, string data, unsigned short seq = 0);
unsigned short to_short(int hi, int lo);
int to_length(int hi, int lo);
string get_recv();




#endif // RDT_H
