#include "NewReno.h"
#include "debug_log.h"
#include "cons.h"
#include <iostream>
#include <time.h>
#include <fstream>

void Runnable(SOCKET server_socket, SOCKADDR_IN client_addr) {
	WriteFile::WriteFileInit(FILE_PATH);
	SendFactory::InitFactory(server_socket, client_addr);
	RecvFactory::InitFactory(server_socket, client_addr);
	unsigned int state = INIT;
	while (state != EXIT) {
		switch (state) {
		case INIT: {
			print_debug(DEBUG_INFO, "state - INIT");
			RecvFactory recv_mess = RecvFactory();
			SendFactory send_mess = SendFactory();
			send_mess.setACK();
			send_mess.setSYN();
			send_mess.send();
			state = SYN_RECV;
			break;
		}
		case SYN_RECV: {
			print_debug(DEBUG_INFO, "state - SYN_RECV");
			RecvFactory recv_mess = RecvFactory();
			state = ESTABLISHED;
			break;
		}
		case ESTABLISHED: {
			print_debug(DEBUG_INFO, "state - ESTABLISHED");
			RecvFactory recv_mess = RecvFactory();
			if (recv_mess.checkFIN()) {
				SendFactory send_mess = SendFactory(recv_mess.getSeqNum());
				send_mess.setACK();
				send_mess.send();
				state = CLOSE_WAIT;
			}
			else if (recv_mess.checkSEN()) {
				SendFactory send_mess = SendFactory(recv_mess.getSeqNum());
				send_mess.setACK();
				send_mess.send();
				expected_seqnum = recv_mess.getSeqNum() + 1;
				state = RECVING;
			}
			break;
		}
		case RECVING: {
			RecvFactory recv_mess = RecvFactory();
			if (!recv_mess.checkCheckNum()) {
				SendFactory send_mess = SendFactory();
				send_mess.setNAK();
				send_mess.setAckNum(expected_seqnum - 1);
				send_mess.send();
				break;
			}
			if (recv_mess.getSeqNum() == expected_seqnum) {
				if(recv_mess.checkFINAL()){
					SendFactory send_mess = SendFactory();
					send_mess.setACK();
					send_mess.setFINAL();
					expected_seqnum++;
					send_mess.setAckNum(expected_seqnum - 1);
					send_mess.send();
					WriteFile::WriteFileClose();
					state = ESTABLISHED;
				}
				else if (recv_mess.checkSEN()) {
					WriteFile::WriteData(recv_mess.getData());
					SendFactory send_mess = SendFactory();
					send_mess.setACK();
					expected_seqnum++;
					send_mess.setAckNum(expected_seqnum - 1);
					send_mess.send();
					//处理recvdata

				}
			}
			else {
				SendFactory send_mess = SendFactory();
				send_mess.setNAK();
				send_mess.setAckNum(expected_seqnum - 1);
				send_mess.send();
			}
			break;
		}
		case CLOSE_WAIT: {
			print_debug(DEBUG_INFO, "state - CLOSE_WAIT");
			SendFactory send_mess = SendFactory();
			send_mess.setFIN();
			send_mess.send();
			state = CLOSED;
			break;
		}
		case CLOSED: {
			print_debug(DEBUG_INFO, "state - CLOSED");
			RecvFactory recv_mess = RecvFactory();
			if (recv_mess.checkFIN()) {
				SendFactory send_mess = SendFactory();
				send_mess.setACK();
				send_mess.send();
				state = CLOSE_WAIT;
				break;
			}
			state = EXIT;
			break;
		}
		}
	}
	return;
}

//静态成员初始化
SOCKET SendFactory::server_socket = 0;
SOCKET RecvFactory::server_socket = 0;
SOCKADDR_IN SendFactory::client_addr = SOCKADDR_IN();
SOCKADDR_IN RecvFactory::client_addr = SOCKADDR_IN();
int SendFactory::StSize = sizeof(client_addr);
int RecvFactory::StSize = sizeof(client_addr);
std::ofstream WriteFile::outfile = std::ofstream();
void SendFactory::InitFactory(SOCKET server_skt, SOCKADDR_IN client_adr) {
	server_socket = server_skt;
	client_addr = client_adr;
	StSize = sizeof(client_addr);
};

SendFactory::SendFactory(unsigned short ack_num)
{
	this->ack_num = ack_num;
	//默认构造
	char zero = 0x00;
	/*长度 16位*/
	int length = 48;
	char len_h = (length & 0xFFFF) >> 8;
	char len_l = (length & 0xFF);
	message += len_h;
	message += len_l;
	/*校验和 预留8位*/
	message += zero;
	/*控制位 预留8位*/
	message += zero;
	/*seq 预留16位*/
	char seq_h = (ack_num & 0xFFFF) >> 8;
	char seq_l = (ack_num & 0xFF);
	message += seq_h;
	message += seq_l;
	return;
}

SendFactory::SendFactory(string data, unsigned short ack_num) {
	//附加数据段
	char zero = 0x00;
	int length = (data.length() * 8) + 48;
	if (length > 65535) {
		print_debug(DEBUG_ERROR, "Message exceeds the maximum length : 65535");
		return;
	}
	/*长度 16位*/
	char len_h = (length & 0xFFFF) >> 8;
	char len_l = (length & 0xFF);
	message += len_h;
	message += len_l;
	/*校验和 预留8位*/
	message += zero;
	/*控制位 预留8位*/
	message += zero;
	/*seq 预留16位*/
	char seq_h = (ack_num & 0xFFFF) >> 8;
	char seq_l = (ack_num & 0xFF);
	message += seq_h;
	message += seq_l;
	message += data;
	this->ack_num = ack_num;
	updateCheckNum();
	return;
};

void SendFactory::updateCheckNum() {
	message[2] = 0;
	int length = message.length();
	short sum = 0x0;
	for (int i = 0; i < length; i++) {
		short cut = message[i];
		if (cut < 0) cut += 256;
		sum += cut;
		check_cr(sum);
	}
	short checknum = (~sum) & (0xFF);
	char check_c = (checknum & 0xFF);
	message[2] = check_c;
	return;
}

void SendFactory::appendData(string data) {
	int length = (data.length() * 8) + message.length();
	if (length > 65535) {
		print_debug(DEBUG_ERROR, "Message exceeds the maximum length : 65535");
		return;
	}
	/*长度 16位*/
	char len_h = (length & 0xFFFF) >> 8;
	char len_l = (length & 0xFF);
	message[0] = len_h;
	message[1] = len_l;
	message += data;
	updateCheckNum();
	return;
}

void SendFactory::send(bool need_log)
{
	sendto(server_socket, message.c_str(), message.length(), 0, (sockaddr*)&client_addr, StSize);
	if (need_log) {
		print_debug_sorv(DEBUG_SEND, message[3], message.length() * 8, this->ack_num);
	}
}

void SendFactory::setAckNum(int ack)
{
	char ack_h = (ack & 0xFFFF) >> 8;
	char ack_l = (ack & 0xFF);
	message[4] = ack_h;
	message[5] = ack_l;
	this->ack_num = ack;
	updateCheckNum();
}

void RecvFactory::InitFactory(SOCKET server_skt, SOCKADDR_IN client_adr)
{
	server_socket = server_skt;
	client_addr = client_adr;
	StSize = sizeof(client_addr);
}

RecvFactory::RecvFactory(bool need_log)
{
	int i = recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &StSize);
	int len_hi = (int)Recv_Buffer[0];
	int len_lo = (int)Recv_Buffer[1];
	if (len_hi < 0) len_hi = 256 + len_hi;
	if (len_lo < 0) len_lo = 256 + len_lo;

	length = len_hi * 0x100 + len_lo * 0x01;
	flags = Recv_Buffer[3];

	short seq_hi = (int)Recv_Buffer[4];
	short seq_lo = (int)Recv_Buffer[5];
	if (seq_hi < 0) seq_hi = 256 + seq_hi;
	if (seq_lo < 0) seq_lo = 256 + seq_lo;

	seq = seq_hi * 0x100 + seq_lo * 0x01;

	int data_length = (length - 48) / 8;
	data = "";
	
	for (int i = 0; i < data_length; i++) {
		data += Recv_Buffer[i + 6];
	}
	
	//check
	short sum = 0x0;
	for (int i = 0; i < length / 8; i++) {
		short cut = (int)Recv_Buffer[i];
		if (cut < 0) cut += 256;
		sum += cut;
		check_cr(sum);
	}
	checked = (sum == 0xFF);
	if (need_log) {
		print_debug_sorv(DEBUG_RECV, flags, length, seq);
	}
}

bool RecvFactory::checkCheckNum()
{
	if(!checked) print_debug(DEBUG_ERROR, "校验和无效");
	return checked;
}

void WriteFile::WriteFileInit(string file_path) {
	outfile = ofstream(file_path, std::ifstream::binary);
}
void WriteFile::WriteData(string data)
{
	outfile.write(data.c_str(), data.length());
}
