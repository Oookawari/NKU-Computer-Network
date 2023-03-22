#include "GBN.h"
#include "cons.h"
#include "debug_log.h"
#include <iostream>
#include <time.h>
#include <fstream>
int stSize = sizeof(CLIENT_SOCKADDR_ADDR);
unsigned short next_ack_num = 0;
void Runnable(SOCKET server_socket, SOCKADDR_IN client_addr) {
	int stSize = sizeof(client_addr);
	std::ofstream outfile;
	outfile = ofstream(FILE_PATH, std::ifstream::binary);
	if (!outfile.is_open())
	{
		print_debug(DEBUG_INFO, "Write File");
		return;
	}
	unsigned int state = INIT;
	while (state != EXIT) {
		switch (state) {
		case INIT: {
			print_debug(DEBUG_INFO, "state - INIT");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));

			state = SYN_RECV;
			string s = "";
			short flag = 0x00 | SYN | ACK;
			s = send(flag, s);
			sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
			print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
			state = SYN_RECV;
			break;
		}
		case SYN_RECV: {
			print_debug(DEBUG_INFO, "state - SYN_RECV");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));

			
			state = ESTABLISHED;
			break;
		}
		case ESTABLISHED: {
			print_debug(DEBUG_INFO, "state - ESTABLISHED");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			short control = Recv_Buffer[3];
			if (control & FIN) {
				string s = "";
				short flag = 0x00 | ACK;

				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = CLOSE_WAIT;
			}
			else if(control & SEN){
				string s = "";
				short flag = 0x00 | ACK;

				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = RECVING;
			}
			break;
		}
		case RECVING: {
			print_debug(DEBUG_INFO, "state - RECVING");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]), to_short((int)Recv_Buffer[4], (int)Recv_Buffer[5]));
			char control = Recv_Buffer[3];
			unsigned short send_seq_num = to_short((int)Recv_Buffer[4], (int)Recv_Buffer[5]);
			/*乱序/出错，不接受*/
			if (!check_checksum(get_recv())) {
				print_debug(DEBUG_ERROR, "state - RECVING | 校验和错误，等待重传");
				string s = "";
				short flag = 0x00 | ACK;
				s = send(flag, s, (next_ack_num - 1));
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8, to_short((int)s[4], (int)s[5]));
				break;
			}
			else if (send_seq_num == next_ack_num) {
				
				int length = to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]);
				length = (length - 48) / 8;
				string data_part = "";
				for (int i = 0; i < length; i++) {
					data_part += Recv_Buffer[i + 6];
				}
				outfile.write(data_part.c_str(), length);
				string s = "";
				short flag = 0x00 | ACK;
				if (control & FINAL) { flag = flag | FINAL; }
				s = send(flag, s, next_ack_num);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8, to_short((int)s[4], (int)s[5]));
				next_ack_num++;
				if (control & FINAL) { 
					state = ESTABLISHED;
				}
			}
			else {
				print_debug(DEBUG_ERROR, "state - RECVING | 乱序，等待重传");
				string s = "";
				short flag = 0x00 | ACK;
				s = send(flag, s, (next_ack_num - 1));
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8, to_short((int)s[4], (int)s[5]));
				break;
			}
			break;
		}
		case CLOSE_WAIT: {
			print_debug(DEBUG_INFO, "state - CLOSE_WAIT");
			string s = "";
			short flag = 0x00 | FIN;
			s = send(flag, s);
			sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
			print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
			state = CLOSED;
			break;
		}
		case CLOSED: {
			print_debug(DEBUG_INFO, "state - CLOSED");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			short control = Recv_Buffer[3];
			if (control & FIN) {
				string s = "";
				short flag = 0x00 | ACK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = CLOSE_WAIT;
				break;
			}
			state = EXIT;
			break;
		}
		}
	}
	outfile.close();
	return;
}

short generate_checksum(string message)
{
	int length = message.length();
	short sum = 0x0;
	for (int i = 0; i < length; i++) {
		short cut = message[i];
		if (cut < 0) cut += 256;//char的范围是-128~127
		sum += cut;
		check_cr(sum);
	}
	short checknum = (~sum) & (0xFF);
	return checknum;
}

bool check_checksum(string message)
{
	int length = message.length();
	short sum = 0x0;
	for (int i = 0; i < length; i++) {
		short cut = message[i];
		if (cut < 0) cut += 256;
		sum += cut;
		check_cr(sum);
	}
	bool crr = (sum == 0xFF);
	if (crr == false) print_debug(DEBUG_ERROR, "校验和无效");
	return crr;
}

string send(short flags, string data, unsigned short ack)
{
	string message = "";
	int length = (data.length() * 8) + 48;
	if (length > 65535) {
		print_debug(DEBUG_ERROR, "Message exceeds the maximum length of 65535");
		return NULL;
	}
	char zero = 0x00;
	char len_h = (length & 0xFFFF) >> 8;
	char len_l = (length & 0xFF);

	/*长度 16位*/
	message += len_h;
	message += len_l;
	
	/*校验和 预留8位*/
	message += zero;

	/*控制位 8位*/
	char flag_c = (flags & 0xFF);
	message += flag_c;
	/*ack 16位*/
	char ack_h = (ack & 0xFFFF) >> 8;
	char ack_l = (ack & 0xFF);

	message += ack_h;
	message += ack_l;
	/*数据段 */
	message += data;

	/*计算校验和*/
	short check = generate_checksum(message);
	char check_c = (check & 0xFF);
	message[2] = check_c;

	return message;
}

int to_length(int hi, int lo)
{
	if (hi < 0) hi = 256 + hi;
	if (lo < 0) lo = 256 + lo;
	int res = hi * 0x100 + lo * 0x01;
	return res;
}
unsigned short to_short(int hi, int lo)
{
	if (hi < 0) hi = 256 + hi;
	if (lo < 0) lo = 256 + lo;
	unsigned short res = hi * 0x100 + lo * 0x01;
	return res;
}
string get_recv()
{
	string mess = "";
	int length = to_length(Recv_Buffer[0], Recv_Buffer[1]) / 8;
	for (int i = 0; i < length; i++) {
		mess += Recv_Buffer[i];
	}
	return mess;
}


