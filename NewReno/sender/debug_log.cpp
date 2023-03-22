#include "debug_log.h"
#include <iostream>
void print_debug(int type, string content)
{
	string printout = "";
	switch (type)
	{
	case DEBUG_INFO:
		printout += "<INFO>\t\t";
		break;
	case DEBUG_ERROR:
		printout += "<ERROR>\t\t";
		break;
	case DEBUG_RECV:
		printout += "<RECV>\t\t";
		break;
	case DEBUG_SEND:
		printout += "<SEND>\t\t";
		break;
	case DEBUG_COMPLETE:
		printout += "<COMPLETE>\t\t";
		break;
	default:
		printout += "<UNKNOWN>\t";
		break;
	}
	printout += content;
	printout += "\n";
	cout << printout;
	return;
}

void print_debug_sorv(int type, char flags, int length, unsigned short ack)
{
	string printout = "";
	switch (type)
	{
	case DEBUG_RECV:
		printout += "<RECV>\t\t";
		break;
	case DEBUG_SEND:
		printout += "<SEND>\t\t";
		break;
	default:
		printout += "<UNKNOWN>\t";
		break;
	}
	printout += "flags : ";
	if (flags & SendFactory::ACK) { printout += " ACK "; }
	if (flags & SendFactory::SYN) { printout += " SYN "; }
	if (flags & SendFactory::FIN) { printout += " FIN "; }
	if (flags & SendFactory::SEN) { printout += " SEN "; }
	if (flags & SendFactory::NAK) { printout += " NAK "; }
	if (flags & SendFactory::FINAL) { printout += " FINAL "; }
	printout += " | ";
	printout += " length : ";
	printout += to_string(length);
	if (type == DEBUG_RECV) {
		printout += " | ack : ";
		printout += to_string(ack);
		printout += "\n";
		cout << printout;
	}
	else {
		printout += " | seq : ";
		printout += to_string(ack);
		printout += "\n";
		cout << printout;
	}
}

void print_debug_windows(int state, int cwnd, int lastByteSent, int lastByteAck, int ssthresh, int dupACKcount)
{
	string printout = "<STATE/WINDOWS>\t";
	switch (state)
	{
	case INIT:
		printout += "state: INIT\t";break;
	case SYN_SENT:
		printout += "state: SYN_SENT\t";break;
	case ESTABLISHED:
		printout += "state: ESTABLISHED\t";break;
	case FIN_WAIT_1:
		printout += "state: FIN_WAIT_1\t";break;
	case FIN_WAIT_2:
		printout += "state: FIN_WAIT_2\t";break;
	case CLOSED:
		printout += "state: CLOSED\t";break;
	case EXIT:
		printout += "state: EXIT\t";break;
	case SLOWSTART:
		printout += "state: SLOWSTART\t";break;
	case CONGESTAVOID:
		printout += "state: CONGESTAVOID\t";break;
	case FASTRECOVER:
		printout += "state: FASTRECOVER\t";break;
	default:
		break;
	}
	printout += "窗口大小: " ;
	printout += to_string(cwnd);
	printout += "\t已发送: ";
	printout += to_string(lastByteSent);
	printout += "\t已确认: ";
	printout += to_string(lastByteAck);
	printout += "\t阈值: ";
	printout += to_string(ssthresh);
	printout += "\t重复ACK数: ";
	printout += to_string(dupACKcount);
	printout += "\n";
	cout << printout;
	return;
}