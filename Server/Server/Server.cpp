#include <iostream>
#include <stack>
#include <deque>
#include <string>
#include <WinSock2.h>
#include <fstream>
#include <time.h>
#include <assert.h>
using namespace std;
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib,"winmm.lib")
#define PORT 90
#define HELPMSG "help - Show Help Menu \r\n" \
                "notepad - Open The Notepad \r\n" \
                "time - Get Local Time \r\n" \
				"chrome - Open The Chrome \r\n" \
                "swap - Swap Mouse Button \r\n" \
                "restore - Restore Mouse Button \r\n" \
				"calculate - Calculate \r\n" \
				"end - End Calculate \r\n" \
                "exit - Quit BdShell"

bool Dispatch(SOCKET, char*);
bool Calculate = FALSE;
/*

i.	客户端敲的字符，必须立即在服务器端及时显示
ii.	可以启动一些应用程序（比如画板、写字板等）
iii.客户端远程登录并连接一台计算机，把要求对方执行的程序（服务器端准备加减乘除四则运算的程序）、参数传给对方，然后执行完毕，把运行结果传回

*/


//判断是否为括号
bool isPra(char ch) {
	if (ch == '(' || ch == ')')
		return true;
	return false;
}

int getPri(char ch) {
	switch (ch) {
	case '+':
	case '-':
		return 0;
		break;
	case '*':
	case '/':
		return 1;
		break;
	case '(':
	case ')':
		return -1;
		break;
	}
}

void check(char ch, stack<char>& coll2, deque<char>& coll3) {
	if (coll2.empty()) {
		coll2.push(ch);
		return;
	}

	if (isPra(ch)) {
		if (ch == '(')
			coll2.push(ch);
		else {
			//弹出所有元素直到遇到左括号
			while (coll2.top() != '(') {
				char t = coll2.top();
				coll3.push_back(t);
				coll2.pop();
			}

			//遇到左括号时，弹出但不加入coll3
			coll2.pop();
		}
	}
	else   //如果不是括号
	{
		//取出栈顶元素，与当前符号进行优先级比较
		char sym = coll2.top();
		if (getPri(ch) <= getPri(sym)) {
			//如果当前pop出的符号sym优先级<=栈顶元素，则弹出
			coll2.pop();
			//压入coll3(后缀表达式)中
			coll3.push_back(sym);
			check(ch, coll2, coll3);
		}
		else {
			//如果ch比栈顶符号优先级大，将c压入coll2(操作符栈)
			coll2.push(ch);
		}
	}
}

void allocate(deque<char>& coll1, stack<char>& coll2, deque<char>& coll3) {
	while (!coll1.empty())
	{
		char c = coll1.front();
		coll1.pop_front();

		if (c >= '0'&&c <= '9')
		{
			coll3.push_back(c);
		}
		else
		{
			//调用check函数，针对不同情况作出不同操作
			check(c, coll2, coll3);
		}

	}

	//如果输入结束，将coll2的元素全部弹出，加入后缀表达式中
	while (!coll2.empty())
	{
		char c = coll2.top();
		coll3.push_back(c);
		coll2.pop();
	}
}



//计算后缀表达式
void calculate(deque<char>& coll3, stack<int>& coll4) {
	while (!coll3.empty())
	{
		char c = coll3.front();
		coll3.pop_front();

		//如果是操作数，压入栈中
		if (c >= '0'&&c <= '9')
		{
			//减去'0'得出偏移值，即为真实数值（如果直接转换成int，结果不对，因为char 转换为int是其编码值，例如'1'的编码值为49
			int op = c - '0';
			coll4.push(op);
		}
		else	 //如果是操作符，从栈中弹出元素进行计算
		{
			int op1 = coll4.top();
			coll4.pop();
			int op2 = coll4.top();
			coll4.pop();
			switch (c)
			{
			case '+':
				coll4.push(op2 + op1);
				break;
			case '-':
				coll4.push(op2 - op1);
				break;
			case '*':
				coll4.push(op2*op1);
				break;
			case '/':
				coll4.push(op2 / op1);  //注意是op2(op)op1而不是op1(op)op2
				break;
			}
		}
	}
}

bool Dispatch(SOCKET socket, char *szCmd) {
	bool bRet = false;
	if (!Calculate) {
		if (!strcmp(szCmd, "help")) {
			send(socket, HELPMSG, strlen(HELPMSG) + sizeof(char), 0);
			bRet = true;
		}
		else if (!strcmp(szCmd, "notepad")) {
			system("notepad");
			send(socket, "Open Notepad Successed!", strlen("Open Notepad Successed!") + sizeof(char), 0);
			//mciSendString(TEXT("set cdaudio door open"), NULL, 0, NULL);

			bRet = true;
		}
		else if (!strcmp(szCmd, "time")) {
			time_t t = time(0);
			char date[64];
			strftime(date, sizeof(date), "%Y年%m月%d日  %H:%M:%S", localtime(&t));
			send(socket, date, strlen(date) + sizeof(char), 0);
			bRet = true;
		}

		else if (!strcmp(szCmd, "chrome")) {
			system("f:\\Chrome.lnk   www.baidu.com");
			send(socket, "Open Chrome Successed!", strlen("Clear Chrome Successed!") + sizeof(char), 0);
			//mciSendString(TEXT("set cdaudio door open"), NULL, 0, NULL);

			bRet = true;
		}

		else if (!strcmp(szCmd, "close")) {
			send(socket, "Close Notepad Sucessed!", strlen("CloseCDoor Sucessed!") + sizeof(char), 0);
			mciSendString(TEXT("set cdaudio door closed"), NULL, 0, NULL);
			bRet = true;
		}
		else if (!strcmp(szCmd, "swap")) {
			send(socket, "MouseButton Changed!", strlen("MouseButton Changed!") + sizeof(char), 0);
			SwapMouseButton(true);
			bRet = true;
		}
		else if (!strcmp(szCmd, "restore")) {
			send(socket, "MouseButton Restored!", strlen("MouseButton Restored!") + sizeof(char), 0);
			SwapMouseButton(false);
			bRet = true;
		}
		else if (!strcmp(szCmd, "calculate")) {
			send(socket, "Please Send the Fornula!", strlen("Please Send the Fornula!") + sizeof(char), 0);
			Calculate = true;
			bRet = true;
		}
	}
	else {
		if (!strcmp(szCmd, "end")) {
			send(socket, "End Calculate", strlen("End Calculate") + sizeof(char), 0);
			Calculate = false;
			bRet = true;
		}
		else {
			char str[30] = "the result is ";
			deque<char> coll1;//中缀表达式
			stack<char> coll2;//操作符
			deque<char> coll3;//后缀表达式
			stack<int> coll4; //计算后缀表达式的辅助容器

			for (int i = 0; i != strlen(szCmd); ++i) {
				if ((szCmd[i] >= '0' && szCmd[i] <= '9') || szCmd[i] == '+' || szCmd[i] == '-' || szCmd[i] == '*' || szCmd[i] == '/' || szCmd[i] == '(' || szCmd[i] == ')') {
					//加入中缀表达式
					coll1.push_back(szCmd[i]);
				}
				else {
					cout << szCmd << "Error" << endl;
					send(socket, "Formula Error", strlen("Formula Error") + sizeof(char), 0);
					bRet = true;
					return bRet;
				}
			}
			//从coll1中取出元素，分配到coll2,coll3中
			allocate(coll1, coll2, coll3);

			//计算后缀表达式
			calculate(coll3, coll4);
			cout << "the result is :" << szCmd << " = " << coll4.top() << endl;

			char strR[30] = {};
			sprintf_s(strR, "%d", coll4.top());
			strcat_s(str, strR);
			send(socket, str, strlen(str) + sizeof(char), 0);
			bRet = true;
		}
	}
	return bRet;
}


bool Login(char *id, char *pwd) {
	fstream fd;
	fd.open("f:\\user.txt");
	if (!fd.is_open()) {
		cout << "用户文件打开失败！\n" << endl;
		system("pause");
		exit(0);
	}
	while (!fd.eof()) {
		char idd[MAXBYTE];
		char pwdd[MAXBYTE];
		while (!fd.eof()) {
			fd.getline(idd, MAXBYTE);
			fd.getline(pwdd, MAXBYTE);
			if (!strcmp(id, idd)) {
				if (!strcmp(pwd, pwdd)) {
					cout << "Login Successful！" << endl;
					fd.close();
					return true;
				}

			}
		}
		cout << "User doesn't Exits！" << endl;
		fd.close();
		return false;
	}
}


int main() {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	//int WSAStartup ( WORD wVersionRequested, LPWSADATA lpWSAData ); 
	//使用Socket的程序在使用Socket之前必须调用WSAStartup函数


	//af : IP 地址类型 -> AF_INET 表示 IPv4 && AF_INET6 表示 IPv6
	//type : 数据传输方式/套接字类型，SOCK_STREAM（流格式套接字/面向连接的套接字）&& SOCK_DGRAM（数据报套接字/无连接的套接字）
	SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN sockaddr;
	/*
	struct sockaddr_in{
	sa_family_t     sin_family;   //地址族（Address Family），也就是地址类型
	uint16_t        sin_port;     //16位的端口号
	struct in_addr  sin_addr;     //32位IP地址
	char            sin_zero[8];  //不使用，一般用0填充
	};
	*/
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sockaddr.sin_port = htons(827);
	//sockaddr_in更细，后14字节:sin_port(2),sin_addr(4),sin_zero(8)
	//sockaddr : sin_data(14),可以表示ipv4和ipv6
	//定义时不适用sockaddr是因为sockaddr初始化时必须同时指明IP地址和端口号，字符串转换相对困难
	bind(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));

	//int listen(SOCKET sock, int backlog);  //Windows
	//sock 为需要进入监听状态的套接字，backlog 为请求队列的最大长度。
	listen(s, 1);

	SOCKADDR clientaddr;
	int nSize = sizeof(SOCKADDR);
	SOCKET clientSock;

	//SOCKET accept(SOCKET sock, struct sockaddr *addr, int *addrlen); 
	//accept() 返回一个新的套接字来和客户端通信，addr 保存了客户端的IP地址和端口号，而 sock 是服务器端的套接字
	clientSock = accept(s, (SOCKADDR*)&clientaddr, &nSize);

	/*

	服务器：socket() -> bind() -> listen() -> accept() ---> 阻塞，等待客户数---------->  read() ---处理服务请求-->  write()  -> close()
																			↑				↑						 ↓
																			↑建立连接		↑请求数据				 ↓应答数据
	客户机:										socket()	----->		 connect()  ->   write()					read()  ->  close()

	*/


	bool flag = false;

	while (1) {
		char id[MAXBYTE] = { 0 };
		char pwd[MAXBYTE] = { 0 };

		send(clientSock, "ID:", strlen("ID:") + sizeof(char), 0);
		recv(clientSock, id, MAXBYTE, 0);
		cout << "input id:" << id << endl;

		send(clientSock, "pwd:", strlen("pwd:") + sizeof(char), 0);
		recv(clientSock, pwd, MAXBYTE, 0);///////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!////////////////////////
		cout << "input pwd:" << pwd << endl;
		
		if (strlen(id) && strlen(pwd))
			flag = Login(id, pwd);
		else
			continue;

		if (flag) {
			send(clientSock, "true", strlen("true")+sizeof(char), 0);
			break;
		}
		else {
			send(clientSock, "false",strlen("false")+sizeof(char), 0);
		}
	}


	while (TRUE) {

		//int send(SOCKET sock, const char *buf, int len, int flags);
		send(clientSock, "BdShell>", strlen("BdShell>") + sizeof(char), 0);
		char buffer[MAXBYTE] = { 0 };
		recv(clientSock, buffer, MAXBYTE, 0);
		cout << buffer << endl;
		if (!strcmp(buffer, "exit"))
			break;

		bool bRet = Dispatch(clientSock, buffer);
		if (bRet == false) {
			send(clientSock, "Failed..", strlen("Failed..") + sizeof(char), 0);

		}
	}
	closesocket(clientSock);
	closesocket(s);

	WSACleanup();
	return 0;
}