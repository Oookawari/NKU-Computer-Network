#ifndef RDT_H
#define RDT_H 1
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
const unsigned int RECV_0 = 6;
const unsigned int RECV_1 = 7;

/*flags*/
const unsigned int ACK = (1 << 7);
const unsigned int SYN = (1 << 6);
const unsigned int FIN = (1 << 5);
const unsigned int ACK_NUM0 = (1 << 4);
const unsigned int ACK_NUM1 = (1 << 3);
const unsigned int NAK = (1 << 2);
const unsigned int FINAL = (1 << 1);

const unsigned int MAX_LENGTH = 65535;
static char Recv_Buffer[MAX_LENGTH];
void Runnable(SOCKET server_socket, SOCKADDR_IN client_addr);

/*Э�鹹�ɣ�
�ļ�ͷ��
���� 16λ(data+16)
У��� 8λ
Ԥ��λ 8λ ��ACK|SYN|FIN|ACK_NUM0|ACK_NUM1|NAK|FINAL|0��
data
*/

/*����У��� 8λ*/
short generate_checksum(string message);
bool check_checksum(string message);
string send(short flags, string data);
int to_length(int hi, int lo);
string get_recv();
#endif // RDT_H
