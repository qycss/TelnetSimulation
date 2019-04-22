#include <iostream>
#include <stdio.h>
#include <WinSock2.h>
#include <conio.h>
#pragma comment(lib,"ws2_32")
using namespace std;

int main(int argc, char *argv[]) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sockaddr.sin_port = htons(827);

	connect(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	bool flag = false;
	char buff[10] = "false";
	while (1) {
		
		char id[MAXBYTE] = { 0 };
		char pwd[MAXBYTE] = { 0 };
		char buff[MAXBYTE] = { 0 };

		recv(s, buff, MAXBYTE, 0);
		cout <<buff;
		cin >> id;
		send(s, id, MAXBYTE, 0);
		recv(s, buff, MAXBYTE, 0);
		cout << buff;
		cin >> pwd;
		send(s, pwd, MAXBYTE, 0);
		recv(s, buff, MAXBYTE, 0);

		if (!strcmp(buff, "true"))
			break;
		cout << "wrong ID or Password! Please try again..." << endl;

	}
	cout << "Login Successful!" << endl;


	while (true) {
		char Buffer[MAXBYTE] = { 0 };
		char Cmd[MAXBYTE] = { 0 };
		recv(s, Buffer, MAXBYTE, 0);
		cout << Buffer;
		cin >> Cmd;
		send(s, Cmd, MAXBYTE, 0);
		if (!strcmp(Cmd, "exit")) {
			cout << "Login out!\r" << endl;
			break;
		}

		memset(Buffer, 0, MAXBYTE);
		recv(s, Buffer, MAXBYTE, 0);
		cout << Buffer << endl;

	}
	closesocket(s);
	WSACleanup();
	system("pause");
	return 0;

}