/*»’÷æ¥Ú”°*/
#ifndef DEBUG_LOG
#define DEBUG_LOG 1
#include<string>
using namespace std;

const int DEBUG_INFO = 0;
const int DEBUG_ERROR = -1;
const int DEBUG_RECV = 1;
const int DEBUG_SEND = 2;
//const int ;

void print_debug(int type, string content);
void print_debug_sorv(int type, char flags, int length, unsigned short seq = 0);

#endif // DEBUG_LOG_H
