/*–≠“È*/
#ifndef PROTOCOL_H
#define PROTOCOL_H 1
#include <string>

/*input list*/
const char I_SEND[] = "send";
const char I_GET[] = "get";
const char I_EXIT[] = "exit";

/*action list*/
const char SEND_USER_NAME[] = "SUN";
const char SEND_TO_USER[] = "STU";
const char REQUEST_USER_LIST[] = "RUL";

/*receive list*/
const char SERVER_SEND_USER_LIST[] = "SUL";
const char SERVER_DELIV_USER_MESS[] = "DUM";
#endif // PROTOCOL_H

