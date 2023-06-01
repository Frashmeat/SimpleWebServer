#pragma once
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>
#include <iterator>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;
// winSokcet 库实现
class TcpListener
{
public:
	TcpListener(const char* ip, int port) :ip(ip), port(port) {};
	~TcpListener() {};
	//初始化 服务器
	int init();
	//开始运行 服务器
	int run();
	void stop(){
		isRun = false;
	}
protected:
	// 当客户端 连接 服务器 时 
	virtual void onClientConnected(SOCKET clientSocket);
	// 当客户端 与服务器 断开连接 时
	virtual void onClientDisconnected(SOCKET clientSocket);
	// 当受到 从客户端的 信息 时
	virtual void onMessageReceived(SOCKET clientSocket, const char* msg, int length);
private:

	void sendToClient(SOCKET clientSocket, const char* msg, int length);
	void broadcastToClients(int sendingClient, const char* msg, int length);
	string createTable(vector<string> data) {
		return "<h1> table <h1>";
	};
	void readImg(ifstream& ifs, string& html,char * data);
	bool isRun = false;
	const char* ip; 
	int port;
	SOCKET m_socket; //用于 服务器监听的 socket 句柄
	fd_set m_master;  //存储 可读写的 socket 句柄 (server 和 client)
};

