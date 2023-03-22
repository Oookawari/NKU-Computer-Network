运行时需要先启动receiver，receiver进入INIT状态后启动sender，连接建立完成后在sender命令行中输入send开始发送文件，文件发送完毕后在sender命令行中输入exit断开连接。

测试程序仅供本地运行，若需要获得可执行程序可能需要重新编译，以下为需要修改的内容：

/sender/cons.h
static const char* SERVER_SOCKADDR_ADDR = "xxx.xxx.xxx.x";替换为自己的ip
static const char* CLIENT_SOCKADDR_ADDR = "xxx.xxx.xxx.x";替换为自己的ip
const int CLI_PORT = 9999;替换为需要的端口
const int SER_PORT = 6666;替换为需要的端口
const string FILE_PATH = "3.jpg";替换为需要发送图片的路径

/receiver/cons.h
static const char* SERVER_SOCKADDR_ADDR = "xxx.xxx.xxx.x";
static const char* CLIENT_SOCKADDR_ADDR = "xxx.xxx.xxx.x";需要与sender中的ip对应。
const int CLI_PORT = 9999;
const int SER_PORT = 6666;需要与sender中的端口对应。
const string FILE_PATH = "3.jpg";替换为需要保存文件的路径

如果不支持<time.h>相关的计时工具：
注释/sender/rdt.cpp所有出现的
struct timespec sts, ets;
timespec_get(&sts, TIME_UTC);
timespec_get(&ets, TIME_UTC);
time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
该四条语句