#ifndef RDT_H
#define RDT_H 1
#include <WinSock2.h>
#include <string>

#define check_cr(sum) {if(sum >= 0x100) sum -= 0xFF;}

using namespace std;
/*state list*/
const unsigned int INIT = 0;
const unsigned int SYN_SENT = 1;
const unsigned int ESTABLISHED = 2;
const unsigned int FIN_WAIT_1 = 3;
const unsigned int FIN_WAIT_2 = 4;
const unsigned int CLOSED = 5;
const unsigned int EXIT = 6;
const unsigned int SEND_0 = 7;
const unsigned int SEND_1 = 8;
const unsigned int W_ACK_0 = 9;
const unsigned int W_ACK_1 = 10;


/*flags*/
const unsigned int ACK = (1 << 7);
const unsigned int SYN = (1 << 6);
const unsigned int FIN = (1 << 5);
const unsigned int SEN_NUM0 = (1 << 4);
const unsigned int SEN_NUM1 = (1 << 3);
const unsigned int NAK = (1 << 2);
const unsigned int FINAL = (1 << 1);

static char* ReadFileBuffer;
static int byte_total;
static int byte_sent;

const unsigned int MAX_LENGTH = 65535;
static char Recv_Buffer[MAX_LENGTH];
static string last_send0;
static string last_send1;

void Runnable(SOCKET client_socket, SOCKADDR_IN server_addr);

/*协议构成：
文件头：
长度 16位(data+16)
校验和 8位
预留位 8位 （ack|syn|fin|0|0|0|0|nextack）
data
*/
/*产生校验和 8位*/
short generate_checksum(string message);
bool check_checksum(string message);
string send(short flags, string data);
string get_data(int start_byte, int length);
string get_recv();
int to_length(int hi, int lo);
#endif // RDT_H
