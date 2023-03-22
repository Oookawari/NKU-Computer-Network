#ifndef NEWRENO_H
#define NEWRENO_H 1
#include <WinSock2.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include "debug_log.h"
#define check_cr(sum) {if(sum >= 0x100) sum -= 0xFF;}

using namespace std;
/*state list*/

static int state;
const unsigned int INIT = 0;
const unsigned int SYN_SENT = 1;
const unsigned int ESTABLISHED = 2;
const unsigned int FIN_WAIT_1 = 3;
const unsigned int FIN_WAIT_2 = 4;
const unsigned int CLOSED = 5;
const unsigned int EXIT = 6;
const unsigned int SLOWSTART = 7;
const unsigned int CONGESTAVOID = 8;
const unsigned int FASTRECOVER = 9;

const unsigned int MAX_LENGTH = 65535;
static char Recv_Buffer[MAX_LENGTH];
void* Recv_Thread(void* param);
void Send_Thread(SOCKET client_socket, SOCKADDR_IN server_addr);

static int lastByteSent = 0;
static int lastByteAck = -1;
static int vectorStart = -1;
static int RACK = -1;
static float cwnd = 0;
static int ssthresh = 64;
static int dupACKcount = 0;

/*并发控制*/
static pthread_mutex_t mutex;
static pthread_rwlock_t mess_vector_lock;
static pthread_rwlock_t send_lock;
//static pthread_rwlock_t mess_vector_lock;
//static pthread_rwlock_t mess_vector_lock;
//static pthread_rwlock_t mess_vector_lock;
static pthread_rwlock_t state_lock;

/*计时*/
static struct timespec sts;
class SendFactory {
private:
	static SOCKET client_socket;
	static SOCKADDR_IN server_addr;
	static int StSize;
	static unsigned short seq_num;
	unsigned short this_seq_num;
	
	static int lost_rate;
	static int wrong_rate;
	struct timespec sts;
public:string message;
	static void InitFactory(SOCKET client_skt, SOCKADDR_IN server_adr);
	static const unsigned int ACK = (1 << 7);
	static const unsigned int SYN = (1 << 6);
	static const unsigned int FIN = (1 << 5);
	static const unsigned int SEN = (1 << 4);
	static const unsigned int NAK = (1 << 2);
	static const unsigned int FINAL = (1 << 1);
	static int getSeq() {
		//返回已使用的seq
		return seq_num - 1;
	};
	static bool countTime(struct timespec sts);
	SendFactory();
	SendFactory(const SendFactory& copy) {
		this->this_seq_num = copy.this_seq_num;
		this->message = copy.message;
		this->sts = copy.sts;
	};
	SendFactory(string data);
	void updateCheckNum(); 
	void setACK() {
		short flags = message[3];
		flags = flags | ACK;
		message[3] = flags;
		updateCheckNum();
		return;
	};
	void setSYN() { 
		short flags = message[3];
		flags = flags | SYN;
		message[3] = flags;
		updateCheckNum();
		return;
	};
	void setSEN() {
		short flags = message[3];
		flags = flags | SEN;
		message[3] = flags;
		updateCheckNum();
		return;
	};
	void setFIN() {
		short flags = message[3];
		flags = flags | FIN;
		message[3] = flags;
		updateCheckNum();
		return;
	};
	void setNAK() {
		short flags = message[3];
		flags = flags | NAK;
		message[3] = flags;
		updateCheckNum();
		return;
	};
	void setFINAL() {
		short flags = message[3];
		flags = flags | FINAL;
		message[3] = flags;
		updateCheckNum();
		return;
	};
	void appendData(string data);
	void send(bool need_log = true);
	void RamdomSend(bool need_log = true);
	
};

class RecvFactory {
private:
	string data;
	char flags;
	unsigned int length;
	unsigned short ack;
	static SOCKET client_socket;
	static SOCKADDR_IN server_addr;
	static int StSize;
	bool checked;
	bool availiable;
public:
	static void InitFactory(SOCKET client_skt, SOCKADDR_IN server_adr);
	static const unsigned int ACK = (1 << 7);
	static const unsigned int SYN = (1 << 6);
	static const unsigned int FIN = (1 << 5);
	static const unsigned int SEN = (1 << 4);
	static const unsigned int NAK = (1 << 2);
	static const unsigned int FINAL = (1 << 1);
	RecvFactory(bool need_log = true);
	int getLength() {
		return length;
	}
	int getAckNum() { 
		return ack; 
	}
	string getData() {
		return data; 
	}
	bool checkACK() {
		return flags & ACK;
	};
	bool checkSYN() {
		return flags & SYN;
	};
	bool checkFIN() {
		return flags & FIN;
	};
	bool checkSEN() {
		return flags & SEN;
	};
	bool checkNAK() {
		return flags & NAK;
	};
	bool checkFINAL() {
		return flags & FINAL;
	};
	bool checkCheckNum();
	bool able() { return this->availiable; }
};

class FileFactory {
private:
	static char* ReadFileBuffer;
	static int byte_total;
	static int byte_sent;
	
public:
	static bool finished;
	static bool FileFinished() { return finished; }
	static void InitFileFactory(string filepath);
	static string getNextData();
	static void Delete() {
		delete[]FileFactory::ReadFileBuffer;
	}
};

typedef struct {
	pthread_t t_handle;
}Thread_Param;

#endif // NEWRENO_H
