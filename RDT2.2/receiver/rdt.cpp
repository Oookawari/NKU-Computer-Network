#include "rdt.h"
#include "cons.h"
#include "debug_log.h"
#include <iostream>
#include <time.h>
#include <fstream>
int stSize = sizeof(CLIENT_SOCKADDR_ADDR);

void Runnable(SOCKET server_socket, SOCKADDR_IN client_addr){
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
			 
			/*为简化过程，直接进入recv0状态*/
			state = RECV_0;
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
			else{}
			break;
		}
		case RECV_0: {
			print_debug(DEBUG_INFO, "state - RECV_0");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			char control = Recv_Buffer[3];
			/*乱序/出错，不接受*/
			if (!check_checksum(get_recv())) {
				print_debug(DEBUG_ERROR, "state - RECV_0 | 校验和错误");
				
				string s = "";
				short flag = 0x00 | ACK_NUM0 | NAK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				break;
			}
			else if (control & ACK_NUM1) {
				print_debug(DEBUG_ERROR, "state - RECV_0 | 乱序错误");
				string s = "";
				short flag = 0x00 | ACK_NUM0 | NAK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				break;
			}
			/*接收完毕*/
			if ((control & FINAL) && (control & ACK_NUM0)) {
				string s = "";
				short flag = 0x00 | FINAL | ACK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = ESTABLISHED;
			}
			/*正常接收*/
			else if ((control & ACK_NUM0)) {

				int length = to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]);
				length = (length - 32) / 8;
				string data_part = "";
				for (int i = 0; i < length; i++) {
					data_part += Recv_Buffer[i + 4];
				}
				outfile.write(data_part.c_str(), length);
				string s = "";
				short flag = 0x00 | ACK_NUM0 | ACK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = RECV_1;
			}
			break; 
		}
		case RECV_1: {
			print_debug(DEBUG_INFO, "state - RECV_1");
			recvfrom(server_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&client_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			short control = Recv_Buffer[3];
			/*乱序/出错，不接受*/
			if (!check_checksum(get_recv())) {
				print_debug(DEBUG_ERROR, "state - RECV_1 | 校验和错误");
				string s = "";
				short flag = 0x00 | ACK_NUM1 | NAK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				break;
			}
			else if ((control & ACK_NUM0)) {

				print_debug(DEBUG_ERROR, "state - RECV_1 | 乱序错误");
				string s = "";
				short flag = 0x00 | ACK_NUM1 | NAK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				break;
			}
			/*接收完毕*/
			if ((control & FINAL) && (control & ACK_NUM1)) {
				string s = "";
				short flag = 0x00 | FINAL | ACK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = ESTABLISHED;
			}
			/*正常接收*/
			else if ((control & ACK_NUM1)) {

				/*读取*/
				int length = to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]);
				length = (length - 32) / 8;
				string data_part = "";
				for (int i = 0; i < length; i++) {
					data_part += Recv_Buffer[i + 4];
				}
				outfile.write(data_part.c_str(), length);
				string s = "";
				short flag = 0x00 | ACK_NUM1 | ACK;
				s = send(flag, s);
				sendto(server_socket, s.c_str(), s.length(), 0, (sockaddr*)&client_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = RECV_0;
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

string send(short flags, string data)
{
	string message = "";
	int length = (data.length() * 8) + 32;
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

string get_recv()
{
	string mess = "";
	int length = to_length(Recv_Buffer[0], Recv_Buffer[1]) / 8;
	for (int i = 0; i < length; i++) {
		mess += Recv_Buffer[i];
	}
	return mess;
}


