#include "debug_log.h"
#include "rdt.h"
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
	cout << printout << endl;
	return;
}

void print_debug_sorv(int type, char flags, int length)
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
	if (flags & ACK) { printout += " ACK "; }
	if (flags & SYN) { printout += " SYN "; }
	if (flags & FIN) { printout += " FIN "; }
	if (flags & SEN_NUM0) { printout += " SEN_NUM0 "; }
	if (flags & SEN_NUM1) { printout += " SEN_NUM1 "; }
	if (flags & NAK) { printout += " NAK "; }
	if (flags & FINAL) { printout += " FINAL "; }
	printout += " | ";
	printout += " length : ";
	cout << printout << length << endl;
	return;
}
