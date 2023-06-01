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
// winSokcet ��ʵ��
class TcpListener
{
public:
	TcpListener(const char* ip, int port) :ip(ip), port(port) {};
	~TcpListener() {};
	//��ʼ�� ������
	int init();
	//��ʼ���� ������
	int run();
	void stop(){
		isRun = false;
	}
protected:
	// ���ͻ��� ���� ������ ʱ 
	virtual void onClientConnected(SOCKET clientSocket);
	// ���ͻ��� ������� �Ͽ����� ʱ
	virtual void onClientDisconnected(SOCKET clientSocket);
	// ���ܵ� �ӿͻ��˵� ��Ϣ ʱ
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
	SOCKET m_socket; //���� ������������ socket ���
	fd_set m_master;  //�洢 �ɶ�д�� socket ��� (server �� client)
};

