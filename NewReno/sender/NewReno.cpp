#include "NewReno.h"
#include "debug_log.h"
#include "cons.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <cstdlib>
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#pragma warning(disable: 4996) 

vector<SendFactory> wait_for_ack;

void Send_Thread(SOCKET client_socket, SOCKADDR_IN server_addr) {
	wait_for_ack.reserve(2000);
	SendFactory::InitFactory(client_socket, server_addr);
	RecvFactory::InitFactory(client_socket, server_addr);
	state = INIT;
	while (state != EXIT) {
		switch (state) {
		case INIT: {
			print_debug(DEBUG_INFO, "state - INIT");
			SendFactory send_mess = SendFactory();
			send_mess.setSYN();
			send_mess.send();
			state = SYN_SENT;
			break;
		}
		case SYN_SENT: {
			print_debug(DEBUG_INFO, "state - SYN_SENT");
			RecvFactory recv_mess = RecvFactory();
			SendFactory send_mess = SendFactory();
			send_mess.setACK();
			send_mess.send();
			state = ESTABLISHED;
			break;
		}
		case ESTABLISHED: {
			print_debug(DEBUG_INFO, "state - ESTABLISHED");
			string action;
			cin >> action;
			if (action == "exit") {
				SendFactory send_mess = SendFactory();
				send_mess.setFIN();
				send_mess.send();
				state = FIN_WAIT_1;
			}
			else if (action == "send") {
				FileFactory::InitFileFactory(FILE_PATH);
				SendFactory send_mess = SendFactory();
				send_mess.setSEN();
				send_mess.send();
				RecvFactory recv_mess = RecvFactory();
				lastByteSent = SendFactory::getSeq();
				vectorStart = lastByteSent;
				lastByteAck = SendFactory::getSeq();
				while (true) {
					string data_part = FileFactory::getNextData();
					if (data_part == "") {
						SendFactory send_mess = SendFactory();
						send_mess.setFINAL();
						wait_for_ack.push_back(send_mess);
						break;
					}
					SendFactory send_mess = SendFactory(data_part);
					send_mess.setSEN();
					wait_for_ack.push_back(send_mess);
				}
				FileFactory::Delete();
				state = SLOWSTART;
			}
			break;
		}
		case SLOWSTART: {
			print_debug_windows(state, cwnd, lastByteSent, lastByteAck, ssthresh, dupACKcount);
			//可发送
			if (FileFactory::finished == true) { Sleep(100);break; }
			if (lastByteSent - lastByteAck <= cwnd) {
				if (lastByteSent - vectorStart == wait_for_ack.size()) {
					FileFactory::finished = true;
					break;
				}
				wait_for_ack[lastByteSent- vectorStart].RamdomSend();
				lastByteSent++;
			}
			else {
				Sleep(100);
			}
			break;
		}
		case CONGESTAVOID: {
			print_debug_windows(state, cwnd, lastByteSent, lastByteAck, ssthresh, dupACKcount);
			if (FileFactory::finished == true) { Sleep(100);break; }
			if (lastByteSent - lastByteAck <= cwnd) {

				if (lastByteSent - vectorStart == wait_for_ack.size()) {
					FileFactory::finished = true;
					break;
				}
				wait_for_ack[lastByteSent - vectorStart].RamdomSend();
				lastByteSent++;
			}
			else {
				Sleep(100);
			}
			break;
		}
		case FASTRECOVER : {
			print_debug_windows(state, cwnd, lastByteSent, lastByteAck, ssthresh, dupACKcount);
			Sleep(100);
			break;
		}
		case FIN_WAIT_1: {
			print_debug(DEBUG_INFO, "state - FIN_WAIT_1");
			RecvFactory recv_mess = RecvFactory();
			if (recv_mess.checkFIN()) {
				state = CLOSED;
			}
			else if (recv_mess.checkACK()) {
				state = FIN_WAIT_2;
			}
			break;
		}
		case FIN_WAIT_2: {
			print_debug(DEBUG_INFO, "state - FIN_WAIT_2");
			RecvFactory recv_mess = RecvFactory();
			if (recv_mess.checkFIN()) {
				state = CLOSED;
			}
			break;
		}
		case CLOSED: {
			print_debug(DEBUG_INFO, "state - CLOSED");
			SendFactory send_mess = SendFactory();
			send_mess.setACK();
			send_mess.send();
			state = EXIT;
			break;
		}
		}
	}
	
	return;
}

void* Recv_Thread(void* param) {
	state = INIT;
	while (state != EXIT) {
		switch (state) {
		case INIT: {
			Sleep(100);
			break;
		}
		case SYN_SENT: {
			Sleep(100);
			break;
		}
		case ESTABLISHED: {
			Sleep(100);
			break;
		}
		case SLOWSTART: {
			RecvFactory recv_mess = RecvFactory();
			if (!recv_mess.able()) {
				if (SendFactory::countTime(sts)) {
					print_debug(DEBUG_ERROR, "相应超时");
					ssthresh = cwnd / 2;
					cwnd = 1;
					dupACKcount = 0;
					for (int i = lastByteAck; i < lastByteSent; i++)
						wait_for_ack[i - vectorStart].send();
					timespec_get(&sts, TIME_UTC);
					state = SLOWSTART;
				}
				break;
			}
			if (!recv_mess.checkCheckNum()) print_debug(DEBUG_ERROR, "接收校验和错误");
			if (recv_mess.checkNAK()) {
				if (recv_mess.getAckNum() == lastByteAck) {
					dupACKcount++;
					if (dupACKcount == 3) {
						print_debug(DEBUG_ERROR, "ACK重复三次，触发快速重传");
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3;
						dupACKcount = 0;
						for (int i = lastByteAck; i < lastByteSent; i++)
							wait_for_ack[i - vectorStart].send();
						RACK = lastByteSent;
						state = FASTRECOVER;
						break;
					}
				}
				if (SendFactory::countTime(sts)) {
					print_debug(DEBUG_ERROR, "响应超时");
					ssthresh = cwnd / 2;
					cwnd = 1;
					dupACKcount = 0;
					for (int i = lastByteAck; i < lastByteSent; i++)
						wait_for_ack[i - vectorStart].send();
					timespec_get(&sts, TIME_UTC);
					state = SLOWSTART;
				}
			}
			else if (recv_mess.checkFINAL()) {
				state = ESTABLISHED;
				break;
			}
			else if (recv_mess.checkACK()) {
				if (recv_mess.getAckNum() > lastByteAck) {
					cwnd++;
					lastByteAck = recv_mess.getAckNum();
					if (cwnd >= ssthresh) { state = CONGESTAVOID; }
					timespec_get(&sts, TIME_UTC);
				}
			}
			break;
		}
		case CONGESTAVOID: {
			RecvFactory recv_mess = RecvFactory();
			if (!recv_mess.able()) {
				if (SendFactory::countTime(sts)) {
					print_debug(DEBUG_ERROR, "相应超时");
					ssthresh = cwnd / 2;
					cwnd = 1;
					dupACKcount = 0;
					for (int i = lastByteAck; i < lastByteSent; i++)
						wait_for_ack[i - vectorStart].send();
					timespec_get(&sts, TIME_UTC);
					state = SLOWSTART;
				}
				break;
			}
			if (!recv_mess.checkCheckNum()) print_debug(DEBUG_ERROR, "接收校验和错误");
			if (recv_mess.checkNAK()) {
				if (recv_mess.getAckNum() < lastByteAck) { break; }
				if (recv_mess.getAckNum() == lastByteAck) {
					dupACKcount++;
					if (dupACKcount == 3) {
						print_debug(DEBUG_ERROR, "ACK重复三次，触发快速重传");
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3;
						dupACKcount = 0;
						RACK = lastByteSent;
						for (int i = lastByteAck; i < lastByteSent; i++)
							wait_for_ack[i - vectorStart].send();
						state = FASTRECOVER;
						break;
					}
				}
				if (SendFactory::countTime(sts)) {
					print_debug(DEBUG_ERROR, "响应超时");
					ssthresh = cwnd / 2;
					cwnd = 1;
					dupACKcount = 0;
					for (int i = lastByteAck; i < lastByteSent; i++)
						wait_for_ack[i - vectorStart].send();
					timespec_get(&sts, TIME_UTC);
					state = SLOWSTART;
				}
			}
			else if (recv_mess.checkFINAL()) {
				state = ESTABLISHED;
				break;
			}
			else if (recv_mess.checkACK()) {
				if (recv_mess.getAckNum() > lastByteAck) {
					dupACKcount = 0;
					float ins = 1 / cwnd;
					cwnd += ins;
					lastByteAck = recv_mess.getAckNum();
					timespec_get(&sts, TIME_UTC);
				}
			}
			break;
		}
		case FASTRECOVER: {
			RecvFactory recv_mess = RecvFactory();
			
			if (!recv_mess.able()) {
				if (SendFactory::countTime(sts)) {
					print_debug(DEBUG_ERROR, "响应超时");
					ssthresh = cwnd / 2;
					cwnd = 1;
					dupACKcount = 0;
					for (int i = lastByteAck; i < lastByteSent; i++)
						wait_for_ack[i - vectorStart].send();
					timespec_get(&sts, TIME_UTC);
					state = SLOWSTART;
				}
				break;
			}
			if (!recv_mess.checkCheckNum()) print_debug(DEBUG_ERROR, "接收校验和错误");
			if (recv_mess.checkNAK()) {
				if (recv_mess.getAckNum() == lastByteAck) {
					cwnd++;
				}
				if (SendFactory::countTime(sts)) {
					print_debug(DEBUG_ERROR, "响应超时");
					ssthresh = cwnd / 2;
					cwnd = 1;
					dupACKcount = 0;

					for (int i = lastByteAck; i < lastByteSent; i++)
						wait_for_ack[i - vectorStart].send();
					timespec_get(&sts, TIME_UTC);
					state = SLOWSTART;
				}
			}
			else if (recv_mess.checkFINAL()) {
				state = ESTABLISHED;
				break;
			}
			else if (recv_mess.checkACK()) {
				if (recv_mess.getAckNum() > lastByteAck) {
					dupACKcount = 0;
					cwnd++;
					lastByteAck = recv_mess.getAckNum();
					if (lastByteAck == RACK) {
						print_debug(DEBUG_INFO, "new ACK -- RACK -- goto CONGESTAVOID");
						cwnd = ssthresh;
						state = CONGESTAVOID;
					}
					else {
						print_debug(DEBUG_INFO, "new ACK -- PACK");
					}
				}
				timespec_get(&sts, TIME_UTC);
			}
			break;
		}
		case FIN_WAIT_1: {
			break;
		}
		case FIN_WAIT_2: {
			break;
		}
		case CLOSED: {
			break;
		}
		}
	}
	return NULL;
}

//静态成员初始化
SOCKET SendFactory::client_socket = 0;
SOCKET RecvFactory::client_socket = 0;
SOCKADDR_IN SendFactory::server_addr = SOCKADDR_IN();
SOCKADDR_IN RecvFactory::server_addr = SOCKADDR_IN();
int SendFactory::StSize = sizeof(server_addr);
int RecvFactory::StSize = sizeof(server_addr);
int SendFactory::lost_rate = 0;
int SendFactory::wrong_rate = 0;
unsigned short SendFactory::seq_num = 0;

char* FileFactory::ReadFileBuffer = nullptr;
int FileFactory::byte_total = 0;
int FileFactory::byte_sent = 0;
bool FileFactory::finished = false;
void SendFactory::InitFactory(SOCKET client_skt, SOCKADDR_IN server_adr) {
	client_socket = client_skt;
	server_addr = server_adr;
	StSize = sizeof(server_addr);
	seq_num = 0;
	lost_rate = LOST_RATE;
	wrong_rate = WRONG_CHECK_RATE;
};

SendFactory::SendFactory()
{
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
	char seq_h = (seq_num & 0xFFFF) >> 8;
	char seq_l = (seq_num & 0xFF);
	message += seq_h;
	message += seq_l;
	this_seq_num = seq_num;
	updateCheckNum();
	seq_num++;
	return;
}

SendFactory::SendFactory(string data) {
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
	char seq_h = (seq_num & 0xFFFF) >> 8;
	char seq_l = (seq_num & 0xFF);
	message += seq_h;
	message += seq_l;
	message += data;
	updateCheckNum();
	this_seq_num = seq_num;
	seq_num++;
	return;
};

void SendFactory::updateCheckNum(){
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
	pthread_rwlock_wrlock(&send_lock);
	sendto(client_socket, message.c_str(), message.length(), 0, (sockaddr*)&server_addr, StSize);
	pthread_rwlock_unlock(&send_lock);
	if (need_log) {
		print_debug_sorv(DEBUG_SEND, message[3], message.length() * 8, this_seq_num);
	}
	timespec_get(&sts, TIME_UTC);
}

void SendFactory::RamdomSend(bool need_log)
{
	int ran = rand() % 100;
	if (ran < lost_rate) {
		print_debug(DEBUG_INFO, "state - SENDING | 触发随机丢包");
	}
	else {
		int ran2 = rand() % 100;
		if (ran2 < wrong_rate) {
			print_debug(DEBUG_INFO, "state - SENDING | 触发校验位出错");
			string wrong_s = message;
			wrong_s[2] = 0xFF;
			pthread_rwlock_wrlock(&send_lock);
			sendto(client_socket, wrong_s.c_str(), wrong_s.length(), 0, (sockaddr*)&server_addr, StSize);
			pthread_rwlock_unlock(&send_lock);
		}
		else { 
			pthread_rwlock_wrlock(&send_lock);
			sendto(client_socket, message.c_str(), message.length(), 0, (sockaddr*)&server_addr, StSize); 
			pthread_rwlock_unlock(&send_lock);
		}
	}
	if (need_log) {
		print_debug_sorv(DEBUG_SEND, message[3], message.length() * 8, this_seq_num);
	}
	timespec_get(&sts, TIME_UTC);
}

bool SendFactory::countTime(struct timespec sts) {
	struct timespec ets;
	timespec_get(&ets, TIME_UTC);
	double time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	return (time_sum > 300);
}

RecvFactory::RecvFactory(bool need_log) {
	int i = recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &StSize);
	if (i == -1) {
		this->availiable = false;
		return;
	}
	this->availiable = true;
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

	ack = seq_hi * 0x100 + seq_lo * 0x01;

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
		print_debug_sorv(DEBUG_RECV, flags, length, ack);
	}
}

void RecvFactory::InitFactory(SOCKET client_skt, SOCKADDR_IN server_adr) {
	client_socket = client_skt;
	server_addr = server_adr;
	StSize = sizeof(server_addr);
};

bool RecvFactory::checkCheckNum() {
	if (!checked) print_debug(DEBUG_ERROR, "校验和无效");
	return checked;
}

void FileFactory::InitFileFactory(string filepath)
{
	ReadFileBuffer = new char[MAX_SIZE];
	fstream file;
	file.open(FILE_PATH, ios::binary | ios::in);
	if (!file.is_open()) {
		print_debug(DEBUG_ERROR, "打开文件错误");
		delete[]ReadFileBuffer;
		return;
	}
	int bytes = 0;
	char byte;
	while (file.read((char*)&byte, sizeof(byte))) {
		ReadFileBuffer[bytes] = byte;
		bytes++;
		if (bytes >= MAX_SIZE) {
			print_debug(DEBUG_ERROR, "文件超过最大长度（100000B）");
			delete[]ReadFileBuffer;
			return;
		}
	}
	byte_total = bytes;
	byte_sent = 0;
}

string FileFactory::getNextData()
{
	int data_length = 0;
	if ((byte_total - byte_sent) > 8185) {
		data_length = 8185;
	}
	else {
		data_length = byte_total - byte_sent;
	}
	
	if (data_length == 0) {
		return "";
	}
	string res = "";
	for (int i = 0; i < data_length; i++) {
		res += ReadFileBuffer[byte_sent + i];
	}
	byte_sent += data_length;
	return res;
}
