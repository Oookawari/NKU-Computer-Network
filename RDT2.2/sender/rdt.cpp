#include "rdt.h"
#include "debug_log.h"
#include "cons.h"
#include <iostream>
#include <fstream>
#include <time.h>

#pragma warning(disable: 4996)

void Runnable(SOCKET client_socket, SOCKADDR_IN server_addr) {
	/*传输字节计数*/
	int bytes_sum = 0;
	struct timespec sts, ets;
	double time_sum = 0.0;
	ReadFileBuffer = new char[MAX_SIZE];
	int stSize = sizeof(server_addr);
	unsigned int state = INIT;
	while (state != EXIT) {
		switch (state) {
		case INIT: {
			print_debug(DEBUG_INFO, "state - INIT");
			string s = "";
			short flag = 0x00 | SYN;
			s = send(flag, s);
			
			sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
			print_debug_sorv(DEBUG_SEND, s[3], s.length()*8);
			state = SYN_SENT;
			break;
		}
		case SYN_SENT: {
			print_debug(DEBUG_INFO, "state - SYN_SENT");
			recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			 
			string s = "";
			short flag = 0x00 | ACK;
			s = send(flag, s);
			 
			sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
			print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
			state = ESTABLISHED;
			break;
		}
		case ESTABLISHED: {
			print_debug(DEBUG_INFO, "state - ESTABLISHED");
			string action;
			cin >> action;
			if (action == "exit") {
				string s = "";
				short flag = 0x00 | FIN;
				s = send(flag, s);
				sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
				state = FIN_WAIT_1;
			}
			else if(action == "send") {
				print_debug(DEBUG_INFO, "file path : ./test.jpg");
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
				timespec_get(&sts, TIME_UTC);
				state = SEND_0;
			}
			break;
		}
		case SEND_0: {
			print_debug(DEBUG_INFO, "state - SEND_0");
			int data_length = 0;
			if ((byte_total - byte_sent) > 8187) {
				data_length = 8187;
			}
			else {
				data_length = byte_total - byte_sent;
			}

			/*已经发送完毕*/
			if (data_length == 0) {
				string s = "";
				short flag = 0x00 | FINAL | SEN_NUM0;
				last_send0 = send(flag, s);
				bytes_sum += last_send0.length();
				sendto(client_socket, last_send0.c_str(), last_send0.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, last_send0[3], last_send0.length() * 8);
			}
			else {
				string s = get_data(byte_sent, data_length);
				byte_sent += data_length;
				short flag = 0x00 | SEN_NUM0;
				last_send0 = send(flag, s);
				bytes_sum += last_send0.length();
				sendto(client_socket, last_send0.c_str(), last_send0.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, last_send0[3], last_send0.length() * 8);
			}
			state = W_ACK_0;
			break;
		}
		case SEND_1: {
			print_debug(DEBUG_INFO, "state - SEND_1");
			int data_length = 0;
			if ((byte_total - byte_sent) > 8187) {
				data_length = 8187;
			}
			else {
				data_length = byte_total - byte_sent;
			}
			/*已经发送完毕*/
			if (data_length == 0) {
				string s = "";
				short flag = 0x00 | FINAL | SEN_NUM1;
				last_send1 = send(flag, s);
				bytes_sum += last_send1.length();
				sendto(client_socket, last_send1.c_str(), last_send1.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, last_send1[3], last_send1.length() * 8);
			}
			else {
				string s = get_data(byte_sent, data_length);
				byte_sent += data_length;
				short flag = 0x00 | SEN_NUM1;
				last_send1 = send(flag, s);

				bytes_sum += last_send1.length();
				sendto(client_socket, last_send1.c_str(), last_send1.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, last_send1[3], last_send1.length() * 8);
			}
			state = W_ACK_1;
			break;
		}
		case W_ACK_0: {
			print_debug(DEBUG_INFO, "state - W_ACK_0");
			recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			short control = Recv_Buffer[3];
			if (!check_checksum(get_recv())) {
				break;
			}
			/*对方不接受（有错/乱序）重传*/
			if (control & NAK) {
				if (control & SEN_NUM0) {
					print_debug(DEBUG_INFO, "重传num0");
					bytes_sum += last_send0.length();
					sendto(client_socket, last_send0.c_str(), last_send0.length(), 0, (sockaddr*)&server_addr, stSize);
					print_debug_sorv(DEBUG_SEND, last_send0[3], last_send0.length() * 8);
				}
				//else if (control & SEN_NUM1) {
					//print_debug(DEBUG_INFO, "重传num1");
					//bytes_sum += last_send1.length();
					//sendto(client_socket, last_send1.c_str(), last_send1.length(), 0, (sockaddr*)&server_addr, stSize);
					//print_debug_sorv(DEBUG_SEND, last_send1[3], last_send1.length() * 8);
				//}
			}
			/*对方接收到发送完毕消息*/
			if ((control & FINAL) && (control & ACK)) {
				state = ESTABLISHED;
				timespec_get(&ets, TIME_UTC);
				time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
				cout << "传输总用时：" << time_sum << " ms" << endl;
				cout << "传输总报文长度：" << bytes_sum << "Bytes" << endl;
				break;
			}
			/*对方ack=0*/
			if ((control & SEN_NUM0) && (control & ACK)) {
				state = SEND_1;
				break;
			}
			break; 
		}
		case W_ACK_1: {
			print_debug(DEBUG_INFO, "state - W_ACK_1");
			recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			short control = Recv_Buffer[3];
			if (!check_checksum(get_recv())) {
				break;
			}
			/*对方不接受（有错/乱序）重传*/
			if ((control & NAK)) {
				if (control & SEN_NUM1) {
					print_debug(DEBUG_INFO, "重传num1");
					bytes_sum += last_send1.length();
					sendto(client_socket, last_send1.c_str(), last_send1.length(), 0, (sockaddr*)&server_addr, stSize);
					print_debug_sorv(DEBUG_SEND, last_send1[3], last_send1.length() * 8);
				}
				//else if (control & SEN_NUM0) {

					//print_debug(DEBUG_INFO, "重传num0");

					//bytes_sum += last_send0.length();
					//sendto(client_socket, last_send0.c_str(), last_send0.length(), 0, (sockaddr*)&server_addr, stSize);
					//print_debug_sorv(DEBUG_SEND, last_send0[3], last_send0.length() * 8);
				//}
			}
			/*对方接收到发送完毕消息*/
			if ((control & FINAL) && (control & ACK)) {
				state = ESTABLISHED;
				timespec_get(&ets, TIME_UTC);
				time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
				cout << "传输总用时：" << time_sum << " ms" << endl;
				cout << "传输总报文长度：" << bytes_sum << "Bytes"<< endl;
				break;
			}
			/*对方ack=1*/
			if ((control & SEN_NUM1) && (control & ACK)) {
				state = SEND_0;
				break;
			}
			break;
		}
		case FIN_WAIT_1: {
			print_debug(DEBUG_INFO, "state - FIN_WAIT_1");
			recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));

			short control = Recv_Buffer[3];
			if ((control & FIN)) {
				state = CLOSED;
			}
			else if (control & ACK) {
				state = FIN_WAIT_2;
			}
			break;
		}
		case FIN_WAIT_2: {
			print_debug(DEBUG_INFO, "state - FIN_WAIT_2");
			recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
			print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
			short control = Recv_Buffer[3];
			if ((control & FIN)) {
				state = CLOSED;
			}
			break;
		}
		case CLOSED: {
			print_debug(DEBUG_INFO, "state - CLOSED");
			string s = "";
			short flag = 0x00 | ACK;
			s = send(flag, s);
			sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
			print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
			state = EXIT;
			break;
		}
		}
	}
	delete[]ReadFileBuffer;
	return;
}

short generate_checksum(string message)
{
	int length = message.length();
	short sum = 0x0;
	for (int i = 0; i < length; i++) {
		short cut = message[i];
		if (cut < 0) cut += 256;
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
	if (crr == false) { print_debug(DEBUG_ERROR, "校验和无效"); }
	return crr;
}

string send(short flags, string data)
{
	string message = "";
	int length = (data.length()*8) + 32;
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

string get_data(int start_byte, int length)
{
	string res = "";
	/*极为粗糙，待改进*/
	for (int i = 0; i < length; i++)
		res += ReadFileBuffer[start_byte + i];
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

int to_length(int hi, int lo)
{
	if (hi < 0) hi = 256 + hi;
	if (lo < 0) lo = 256 + lo;
	int res = hi * 0x100 + lo * 0x01;
	return res;
}