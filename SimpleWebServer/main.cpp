#include<iostream>
#include"TcpListener.h"
using namespace std;

int main() {

	TcpListener tl("127.0.0.1",8080);
	tl.init();
	cout << tl.run() << endl;
	system("pause");


	return 0;
}