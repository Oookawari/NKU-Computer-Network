#ifndef GBN_H
#define GBN_H 1
#include <WinSock2.h>
#include <string>
#include <iostream>
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

const unsigned int MAX_LENGTH = 65535;
static char Recv_Buffer[MAX_LENGTH];
void Runnable(SOCKET server_socket, SOCKADDR_IN client_addr);

static int expected_seqnum = 0;

class SendFactory {
private:
	static SOCKET server_socket;
	static SOCKADDR_IN client_addr;
	static int StSize;
	unsigned short ack_num;
	string message;
public:
	static void InitFactory(SOCKET server_skt, SOCKADDR_IN client_adr);
	static const unsigned int ACK = (1 << 7);
	static const unsigned int SYN = (1 << 6);
	static const unsigned int FIN = (1 << 5);
	static const unsigned int SEN = (1 << 4);
	static const unsigned int NAK = (1 << 2);
	static const unsigned int FINAL = (1 << 1);
	SendFactory(unsigned short ack_num = 0);
	SendFactory(string data, unsigned short ack_num = 0);
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
	void setAckNum(int ack);
};

class RecvFactory {
private:
	string data;
	char flags;
	unsigned int length;
	unsigned short seq;
	static SOCKET server_socket;
	static SOCKADDR_IN client_addr;
	static int StSize;
	bool checked;
public:
	static void InitFactory(SOCKET server_skt, SOCKADDR_IN client_adr);
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
	int getSeqNum() {
		return seq;
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
};

class WriteFile {
private:
	static std::ofstream outfile;
public:
	static void WriteFileInit(string file_path);
	static void WriteData(string data);
	static void WriteFileClose() { /*outfile.close();*/ };
};
#endif // RDT_H
