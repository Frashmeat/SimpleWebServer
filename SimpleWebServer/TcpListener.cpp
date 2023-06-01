#include "TcpListener.h"
//#define DEBUG 
//#define Project5
int TcpListener::init()
{	
	//ȷ�� WSA �汾 �� ���� dll
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2); // ȷ�� WSA �İ汾
	int isWSADone = WSAStartup(ver, &wsData); // ���� WSA �� dll
	if (isWSADone != 0) { // ����ʧ��ֱ�ӷ���
		return isWSADone;
	}


#ifdef DEBUG 
	std::cout << "WSA done\n";
#endif // DEBUG

	// socket ����
	m_socket = socket(AF_INET, SOCK_STREAM, 0); // ���� socket ���
	if (m_socket == INVALID_SOCKET) return WSAGetLastError(); // ����ʧ�� 

#ifdef DEBUG
	std::cout << "Sokcet done\n";
#endif // DEBUG
	//��ip ,port �� socket
	sockaddr_in hint;  
	hint.sin_family = AF_INET; // ָ��Э����
	hint.sin_port = htons(port); //ָ���˿�
	inet_pton(AF_INET, ip, &hint.sin_addr); //��ip �� sin_addr
	// ִ�а�
	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		return WSAGetLastError(); // ��ʧ��
	}

#ifdef DEBUG
	std::cout << "Sokcet binded\n";
#endif // DEBUG

	//��ʼ���� client ����
	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
		//��������ʧ�� ֱ��return
		return WSAGetLastError();
	}


	//��� FD_SET ȷ�� �ʼֻ�� ���ڼ����� m_socket
	FD_ZERO(&m_master);
	// ��m_socket ���� m_master  
	FD_SET(m_socket, &m_master);

	return 0;
}

int TcpListener::run()
{

	isRun = true;
	cout << "server start ... \n";

	while (isRun) {
		//���� fd_set ��copy  ���� ֮��Ҫ����select���� ������ �Ƿ���socket��״̬�仯  select ���ܻ�Դ����set �����޸�
		fd_set copy = m_master;
		//���� �Ƿ� �� �����״̬ ���� �仯
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
			// �Ƿ� �� �µ� client ����
			if (socket == m_socket) {
				SOCKET client = accept(m_socket, nullptr, nullptr);
				if (client != SOCKET_ERROR) {
					FD_SET(client, &m_master);
					//���� ���ͻ��� ����ʱ �� ����
					onClientConnected(client);
				}
			}
			else { //��client socket �ڷ�����Ϣ
				char buf[4096];
				ZeroMemory(buf, 4096);

				int data = recv(socket, buf, 4096, 0);
				if (data <= 0) { // û�� �յ� ��Ϣ
					 // �ͻ��������Ͽ�, ����ɾ��
					onClientDisconnected(socket);
					closesocket(socket);
					FD_CLR(socket, &m_master);
				}
				else { // �з�����Ϣ
					onMessageReceived(socket, buf, data);
					//cout << '\n';
				}
			}
		}
		//cout << m_master.fd_count << '\n';
	}

	//����ѭ�� 
	//��� socket ���
	FD_CLR(m_socket, &m_master);
	closesocket(m_socket);
	while (m_master.fd_count > 0) {
		SOCKET sock = m_master.fd_array[0];
		FD_CLR(sock, &m_master);
		closesocket(sock);
	}


	// �ر� winsock ��������
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
{	//	����һ��string�� ������
	std::istringstream iss(msg);
	// �����յ� ����Ϣ ���� װ�� vector
	std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

	// Ĭ�Ϸ��ʵ��趨 (404 file not found 'page')
	std::string content = 
		"<h1>404 Not Found !</h1>"; // ��д 404 html �ļ�
	std::string htmlFile = "index.html"; // �ڱ��ص� html �ļ�
	int errorCode = 404; // ״̬��
	
#ifdef DEBUG
	
#endif // DEBUG
	/*for (string& temp : parsed) {
		cout << temp<<'\n';
}*/
	vector<string> data;
	//�ж� ���� ����Ϣ�Ƿ� ��Ч ͬʱ �ж� ���͵ķ�ʽ�Ƿ��� "GET"
	if (parsed.size() >= 3 && parsed[0] == "GET")
	{
		
		htmlFile = parsed[1];
		//cout << htmlFile << endl;
		//�Ƿ���ֱ�ӷ��� "127.0.0.1:8080"

		if (htmlFile == "/")
		{
			htmlFile = "index.html";
		}
		else if (htmlFile.substr(htmlFile.size() - 3, htmlFile.size()) == "jpg") {
			htmlFile = "image/picture.jpg";
			ifstream stream(htmlFile, ios::out | ios::binary);
			int len = 0;
			char* data = nullptr;
			if (stream.is_open()) {
				stream.seekg(0, std::ios::end);
				len = stream.tellg();
				stream.seekg(0, std::ios::beg);
				data = new char[len];
				stream.read(data, len);
				cout << "done" << endl;
			}
			errorCode = 200;
			std::ostringstream oss;
			oss << "HTTP/1.1 " << errorCode << " OK\r\n";
			//oss << "Accept-Ranges: bytes\r\n";
			oss << "Content-Type: image/jpg\r\n";
			oss << "Content-Length: " << len << "\r\n";
			oss << "\r\n";
			/*for (int i = 0; i < len; i++) {
				oss << data[i];
			}*/
			oss << string(data, len);

			std::string output = oss.str();
			sendToClient(clientSocket, output.c_str(), output.size());
			return;
		} // 123.jpg 7 -3 = 4
#ifdef Project5

		else {
			//127.0.0.1:8080?username=11&tel=11&age=1&sex=1&clicked=�ύ
			for (int i = 0; i < htmlFile.size(); i++) {
				if (htmlFile[i] == '=') {
					int j = i;
					while (htmlFile[j] != '&'&&j!=htmlFile.size()-1) {
						j++;
					}
					string dataPart;
					if (j != htmlFile.size() - 1)
						dataPart = htmlFile.substr(i + 1, j - i - 1);
					/*else
						dataPart = htmlFile.substr(i + 1, j - i);*/
					//cout << dataPart << endl;
					if(dataPart != "")
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
		o.open("data.txt",ios::app);
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

		


		// ���� ���ص� html �ļ�
		ifstream f(htmlFile);

		//	���سɹ� ���� html �ļ�
		if (f.good())
		{
			std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
			content = str;
			errorCode = 200;
		}

		f.close();


		std::ostringstream oss;

		oss << "HTTP/1.1 " << errorCode << " OK\r\n";
		//oss << "Cache-Control: no-cache, private\r\n";
		oss << "Content-Type: text/html\r\n";
		oss << "Content-Length: " << content.size() << "\r\n";
		oss << "\r\n";
		oss << content;

		std::string output = oss.str();
		//int size = output.size() + 1;

		sendToClient(clientSocket, output.c_str(), output.size() + 1);
	}
	