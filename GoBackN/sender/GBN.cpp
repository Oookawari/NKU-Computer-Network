#include "GBN.h"
#include "debug_log.h"
#include "cons.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <queue>
#include <cstdlib>
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#pragma warning(disable: 4996)

/*序列号*/
int seq_num = 0;
/*对方已确认的序列号*/
unsigned short next_ack_num = 0;
bool finished = false;

/*已发送 未确认*/
queue<string> wait_ack_buffer;
queue<struct timespec> timer_buffer;
struct timespec sts, ets;
struct timespec sts_sum, ets_sum;
double time_sum = 0.0;
double all_time_sum = 0.0;
unsigned short last_ack = 0;
unsigned int state = INIT;
int rept = 0;

/*并发控制*/
pthread_rwlock_t mess_queue_lock;
pthread_rwlock_t state_lock;
pthread_rwlock_t timer_queue_lock;
pthread_rwlock_t next_ack_num_lock;

void Runnable(SOCKET client_socket, SOCKADDR_IN server_addr) {
	/*传输字节计数*/
	int bytes_sum = 0;
	struct timespec sts, ets;
	double time_sum = 0.0;
	ReadFileBuffer = new char[MAX_SIZE];
	int stSize = sizeof(server_addr);
	while (state != EXIT) {
		switch (state) {
		case INIT: {
			print_debug(DEBUG_INFO, "state - INIT");
			string s = "";
			short flag = 0x00 | SYN;
			s = send(flag, s);

			sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
			print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);
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
			else if (action == "send") {
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
				string s = "";
				short flag = 0x00 | SEN;
				s = send(flag, s);
				sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8);

				recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
				print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]));
				finished = false;
				seq_num = 0;
				next_ack_num = 0;
				state = SENDING;
				//线程参数
				Thread_Param t_param;
				t_param.client_socket = client_socket;
				t_param.server_addr = server_addr;
				t_param.t_handle;
				//线程锁初始化
				pthread_rwlock_init(&mess_queue_lock, NULL);
				pthread_rwlock_init(&state_lock, NULL);
				pthread_rwlock_init(&timer_queue_lock, NULL);
				pthread_rwlock_init(&next_ack_num_lock, NULL);
				pthread_create(&(t_param.t_handle), NULL, Recv_Thread, (void*)&t_param);
				timespec_get(&sts_sum, TIME_UTC);
			}
			break;
		}
		case SENDING: {
			//print_debug(DEBUG_INFO, "state - SENDING");
			//窗口信息输出：
			string debug_mess = "";
			debug_mess += "---base:  " + to_string(next_ack_num - 1) + " --- nextseqnum: " + to_string(seq_num) +
				+ "---待确认: " + to_string(wait_ack_buffer.size()) + "\n";
			cout << debug_mess;

			if (!finished && wait_ack_buffer.size() < WINDOW_SIZE) {
				string s = get_data();
				pthread_rwlock_wrlock(&mess_queue_lock);
				wait_ack_buffer.push(s);
				pthread_rwlock_unlock(&mess_queue_lock);
				//随机丢包发送
				random_sent(s,client_socket, server_addr);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8, to_short((int)s[4], (int)s[5]));
				timespec_get(&sts, TIME_UTC);
			}
			else {
				state = WINDOWSFULL;
			}
			break;
		}
		case WINDOWSFULL: {
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

string send(short flags, string data, unsigned short seq)
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

	/*seq 16位*/
	char seq_h = (seq & 0xFFFF) >> 8;
	char seq_l = (seq & 0xFF);

	message += seq_h;
	message += seq_l;
	/*数据段 */
	message += data;

	/*计算校验和*/
	short check = generate_checksum(message);
	char check_c = (check & 0xFF);
	message[2] = check_c;

	return message;
}

string get_data()
{
	int data_length = 0;
	if ((byte_total - byte_sent) > 8185) {
		data_length = 8185;
	}
	else {
		data_length = byte_total - byte_sent;
	}
	/*如果已经全部发送完毕，包装最后一个报文并不再进入该函数*/
	if (data_length == 0) {
		string s = "";
		short flag = 0x00 | FINAL ;
		s = send(flag, s, seq_num);
		seq_num++;
		finished = true;
		return s;
	}
	string res = "";
	for (int i = 0; i < data_length; i++)
		res += ReadFileBuffer[byte_sent + i];
	byte_sent += data_length;
	short flag = 0x00 | SEN;

	
	res = send(flag, res, seq_num);
	seq_num++;
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

unsigned short to_short(int hi, int lo)
{
	if (hi < 0) hi = 256 + hi;
	if (lo < 0) lo = 256 + lo;
	unsigned short res = hi * 0x100 + lo * 0x01;
	return res;
}

int to_length(int hi, int lo)
{
	if (hi < 0) hi = 256 + hi;
	if (lo < 0) lo = 256 + lo;
	int res = hi * 0x100 + lo * 0x01;
	return res;
}

void random_sent(string s,SOCKET client_socket, SOCKADDR_IN server_addr)
{
	int stSize = sizeof(server_addr);
	int ran = rand() % 100;
	if (ran < LOST_RATE) { 
		print_debug(DEBUG_INFO, "state - SENDING | 触发随机丢包"); 
	}
	else {
		int ran2 = rand() % 100;
		if (ran2 < WRONG_CHECK_RATE) {
			print_debug(DEBUG_INFO, "state - SENDING | 触发校验位出错");
			string wrong_s = s;
			wrong_s[2] = 0xFF;
			sendto(client_socket, wrong_s.c_str(), wrong_s.length(), 0, (sockaddr*)&server_addr, stSize);
		}
		else { sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize); }
	}
}

void* Recv_Thread(void* param) {
	Thread_Param* p = (Thread_Param*)param;
	SOCKET client_socket = p->client_socket;
	SOCKADDR_IN server_addr = p->server_addr;
	int stSize = sizeof(server_addr);
	while (1) {
		recvfrom(client_socket, Recv_Buffer, MAX_LENGTH, 0, (sockaddr*)&server_addr, &stSize);
		print_debug_sorv(DEBUG_RECV, Recv_Buffer[3], to_length((int)Recv_Buffer[0], (int)Recv_Buffer[1]), to_short((int)Recv_Buffer[4], (int)Recv_Buffer[5]));
		short control = Recv_Buffer[3];
		if (!check_checksum(get_recv())) {
			print_debug(DEBUG_ERROR, "state - SENDING 校验和错误");
			continue;
		}
		/*对方已接收结束消息*/
		if ((control & ACK) && (control & FINAL)) {
			timespec_get(&ets_sum, TIME_UTC);
			all_time_sum = (ets_sum.tv_nsec - sts_sum.tv_nsec) * 0.000001 + (ets_sum.tv_sec - sts_sum.tv_sec) * 1000;
			cout << "传输字节数：" << byte_total << endl;
			cout << "传输用时：" << all_time_sum << endl;
			/*清空队列*/
			pthread_rwlock_wrlock(&mess_queue_lock);
			while (wait_ack_buffer.size() != 0) {
				wait_ack_buffer.pop();
			}
			state = ESTABLISHED;
			pthread_rwlock_unlock(&mess_queue_lock);
			break;
		};
		unsigned short recv_ack_num = to_short((int)Recv_Buffer[4], (int)Recv_Buffer[5]);
		/*ack序号正确*/
		if (recv_ack_num == next_ack_num) {
			next_ack_num++;
			/*弹出已确认的报文*/
			//对变量加锁
			pthread_rwlock_wrlock(&mess_queue_lock);
			wait_ack_buffer.pop();
			pthread_rwlock_unlock(&mess_queue_lock);
			timespec_get(&sts, TIME_UTC);
			state = SENDING;
			continue;
		}
		timespec_get(&ets, TIME_UTC);
		time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
		if (time_sum > 200) {
			print_debug(DEBUG_INFO, "state - SENDING | ack超时，重发");
			/*将队列中所有报文重发*/
			int que_num = wait_ack_buffer.size();
			pthread_rwlock_wrlock(&mess_queue_lock);
			for (int i = 0; i < que_num; i++) {
				string s = wait_ack_buffer.front();
				sendto(client_socket, s.c_str(), s.length(), 0, (sockaddr*)&server_addr, stSize);
				print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8, to_short((int)s[4], (int)s[5]));
				wait_ack_buffer.pop();
				wait_ack_buffer.push(s);
			}
			pthread_rwlock_unlock(&mess_queue_lock);
		}
	}
	return NULL;
}

case SENDING: {
	//print_debug(DEBUG_INFO, "state - SENDING");
	//窗口信息输出：
	string debug_mess = "";
	debug_mess += "---base:  " + to_string(next_ack_num - 1) + " --- nextseqnum: " + to_string(seq_num) +
		+"---待确认: " + to_string(wait_ack_buffer.size()) + "\n";
	cout << debug_mess;

	if (!finished && wait_ack_buffer.size() < WINDOW_SIZE) {
		string s = get_data();
		pthread_rwlock_wrlock(&mess_queue_lock);
		wait_ack_buffer.push(s);
		pthread_rwlock_unlock(&mess_queue_lock);
		//随机丢包发送
		random_sent(s, client_socket, server_addr);
		print_debug_sorv(DEBUG_SEND, s[3], s.length() * 8, to_short((int)s[4], (int)s[5]));
		timespec_get(&sts, TIME_UTC);
	}
	else {
		state = WINDOWSFULL;
	}
	break;
}
case WINDOWSFULL: {
	break;
}