#include "TcpListener.h"
//#define DEBUG 
//#define Project5
int TcpListener::init()
{	
	//确定 WSA 版本 和 加载 dll
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2); // 确定 WSA 的版本
	int isWSADone = WSAStartup(ver, &wsData); // 加载 WSA 的 dll
	if (isWSADone != 0) { // 加载失败直接返回
		return isWSADone;
	}


#ifdef DEBUG 
	std::cout << "WSA done\n";
#endif // DEBUG

	// socket 创建
	m_socket = socket(AF_INET, SOCK_STREAM, 0); // 创建 socket 句柄
	if (m_socket == INVALID_SOCKET) return WSAGetLastError(); // 创建失败 

#ifdef DEBUG
	std::cout << "Sokcet done\n";
#endif // DEBUG
	//绑定ip ,port 到 socket
	sockaddr_in hint;  
	hint.sin_family = AF_INET; // 指定协议族
	hint.sin_port = htons(port); //指定端口
	inet_pton(AF_INET, ip, &hint.sin_addr); //绑定ip 到 sin_addr
	// 执行绑定
	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		return WSAGetLastError(); // 绑定失败
	}

#ifdef DEBUG
	std::cout << "Sokcet binded\n";
#endif // DEBUG

	//开始监听 client 连接
	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
		//开启监听失败 直接return
		return WSAGetLastError();
	}


	//清空 FD_SET 确保 最开始只有 用于监听的 m_socket
	FD_ZERO(&m_master);
	// 将m_socket 加入 m_master  
	FD_SET(m_socket, &m_master);

	return 0;
}

int TcpListener::run()
{

	isRun = true;
	cout << "server start ... \n";

	while (isRun) {
		//创建 fd_set 的copy  由于 之后要调用select函数 来监听 是否有socket的状态变化  select 可能会对传入的set 进行修改
		fd_set copy = m_master;
		//监听 是否 有 句柄的状态 发生 变化
#ifdef DEBUG
		std::cout << "select ready\n";
#endif // DEBUG
		//std::cout << "server begin\n";
		int whichSocketChange = select(copy.fd_count, &copy, nullptr, nullptr, nullptr);
#ifdef DEBUG
		//std::cout << m_master.fd_count << "\n";
		std::cout << "select done\n\n\n";
#endif // DEBUG
		//cout << whichSocketChange << endl;
		for (int i = 0; i < whichSocketChange; i++)
		{
			SOCKET socket = copy.fd_array[i];
			// 是否 有 新的 client 连接
			if (socket == m_socket) {
				SOCKET client = accept(m_socket, nullptr, nullptr);
				if (client != SOCKET_ERROR) {
					FD_SET(client, &m_master);
					//调用 当客户端 连接时 的 方法
					onClientConnected(client);
				}
			}
			else { //是client socket 在发送消息
				char buf[4096];
				ZeroMemory(buf, 4096);

				int data = recv(socket, buf, 4096, 0);
				if (data <= 0) { // 没有 收到 消息
					 // 客户端正常断开, 进行删除
					onClientDisconnected(socket);
					closesocket(socket);
					FD_CLR(socket, &m_master);
				}
				else { // 有发送消息
					onMessageReceived(socket, buf, data);
					//cout << '\n';
				}
			}
		}
		//cout << m_master.fd_count << '\n';
	}

	//跳出循环 
	//清除 socket 句柄
	FD_CLR(m_socket, &m_master);
	closesocket(m_socket);
	while (m_master.fd_count > 0) {
		SOCKET sock = m_master.fd_array[0];
		FD_CLR(sock, &m_master);
		closesocket(sock);
	}


	// 关闭 winsock 结束运行
	WSACleanup();
	return 0;
}

void TcpListener::sendToClient(SOCKET clientSocket, const char* msg, int length)
{
	send(clientSocket, msg, length, 0);
}
void TcpListener::broadcastToClients(int sendingClient, const char* msg, int length)
{
	for (int i = 0; i < m_master.fd_count; i++)
	{
		if (m_master.fd_array[i] != m_socket && m_master.fd_array[i] != sendingClient) {
			sendToClient(m_master.fd_array[i], msg, length);
		}
	}
}



void TcpListener::onClientConnected(SOCKET clientSocket)
{
	cout<< "client :" << clientSocket << '\n';
}
void TcpListener::onClientDisconnected(SOCKET clientSocket)
{
	cout << "client :" << clientSocket << " disconnected " << '\n';
}
void TcpListener::onMessageReceived(SOCKET clientSocket, const char* msg, int length)
{	
	//	创建一个string的 输入流
	std::istringstream iss(msg);
	// 将接收到 的信息 按行 装入 vector
	std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

	// 默认访问的设定 (404 file not found 'page')
	std::string content =
		"<h1>404 Not Found !</h1>"; // 手写 404 html 文件
	std::string htmlFile = "index.html"; // 在本地的 html 文件
	int errorCode = 404; // 状态码

#ifdef DEBUG

#endif // DEBUG
	/*for (string& temp : parsed) {
		cout << temp<<'\n';
}*/
	vector<string> data;
	//判断 接收 的信息是否 有效 同时 判断 发送的方式是否是 "GET"
	if (parsed.size() >= 3 && parsed[0] == "GET")
	{
		htmlFile = parsed[1];
		//cout << htmlFile << endl;
		//是否是直接访问 "127.0.0.1:8080"

		if (htmlFile == "/")
		{
			htmlFile = "index.html";
			// 加载 本地的 html 文件
			ifstream f(htmlFile);

			//	加载成功 返回 html 文件
			if (f.good())
			{
				std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
				content = str;
				errorCode = 200;
			}
			f.close();
			//准备数据

			std::string output = addResposeHeader(errorCode, "text/html", content.c_str(), content.length());
			sendToClient(clientSocket, output.c_str(), output.size() + 1);
			return;
		}
		else if (htmlFile.substr(htmlFile.size() - 3, htmlFile.size()) == "jpg") {

			htmlFile = "image/picture.jpg";
			char* data = nullptr;
			int len = readImg(htmlFile, data);
			errorCode = 200;
			std::string output = addResposeHeader(errorCode, "image/jpg", data, len);
			sendToClient(clientSocket, output.c_str(), output.size());
			return;

		}
		else if (htmlFile.substr(htmlFile.size() - 3, htmlFile.size()) == "css") {
			htmlFile = "test.css";
			ifstream f(htmlFile);

			//	加载成功 返回 html 文件
			if (f.good())
			{
				std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
				content = str;
				errorCode = 200;
			}
			f.close();
			//准备数据

			std::string output = addResposeHeader(errorCode, "text/css", content.c_str(), content.length());
			sendToClient(clientSocket, output.c_str(), output.size() + 1);
			return;
		}
#ifdef Project5

		else {
			//127.0.0.1:8080?username=11&tel=11&age=1&sex=1&clicked=提交
			for (int i = 0; i < htmlFile.size(); i++) {
				if (htmlFile[i] == '=') {
					int j = i;
					while (htmlFile[j] != '&' && j != htmlFile.size() - 1) {
						j++;
					}
					string dataPart;
					if (j != htmlFile.size() - 1)
						dataPart = htmlFile.substr(i + 1, j - i - 1);
					/*else
						dataPart = htmlFile.substr(i + 1, j - i);*/
						//cout << dataPart << endl;
					if (dataPart != "")
						data.push_back(dataPart);
				}
			}

			errorCode = 200;

		}
#endif // Project5
	}
#ifdef Project5

	if (!data.empty()) {
		ofstream o;
		o.open("data.txt", ios::app);
		o << '\n';
		for (string& temp : data) {
			o << temp << " ";
		}
		o.close();
		string str;
		ifstream ifs("data.txt");

		htmlFile = createTable(data);
		content = htmlFile;
	}
#endif // Project5
	string output = addResposeHeader(errorCode, "text/html", content.c_str(), content.length());
	sendToClient(clientSocket, output.c_str(), output.size());
		
}
	vector<string> TcpListener::split(const string& str, const string& pattern)
{
	std::vector<std::string> res;
	if (str == "")
		return res;
	std::string strs = str + pattern;
	size_t pos = strs.find(pattern);

	while (pos != strs.npos)
	{
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(pattern);
	}

	return res;
}
	int TcpListener::readImg(string& htmlFile, char*& data)
	{
		ifstream stream(htmlFile, ios::out | ios::binary);
		int len = 0;
		data = nullptr;
		if (stream.is_open()) {
			stream.seekg(0, std::ios::end);
			len = stream.tellg();
			stream.seekg(0, std::ios::beg);
			data = new char[len];
			stream.read(data, len);
			cout << "done" << endl;
		}
		return len;
	}
	string TcpListener::addResposeHeader(int errorCode, string&& contentType,const char* data,int len)
	{
		ostringstream oss;
		oss << "HTTP/1.1 " << errorCode << " OK\r\n";
		//oss << "Cache-Control: no-cache, private\r\n";
		oss << "Content-Type:" << contentType <<"\r\n";
		oss << "Content-Length: " << len << "\r\n";
		oss << "\r\n";
		if (contentType == "image/jpg")
		{
			oss << string(data, len);
		}
		else {
			oss << data ;
		}
		return oss.str();
	}
