#include <winsock2.h>
#include <Windows.h>

#include <stdio.h>
#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")


int make_server_socket(int port)
{

	WSADATA inet_WsaData;
	WSAStartup(MAKEWORD(2, 0), &inet_WsaData);
	if (LOBYTE(inet_WsaData.wVersion) != 2 || HIBYTE(inet_WsaData.wVersion) != 0)
	{
		WSACleanup();
		return -1;
	}
	int tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	//saddr.sin_addr.s_addr = INADDR_ANY;//监听任意地址
	saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//只监听本机

	if (::bind(tcp_socket, (const struct sockaddr*)&saddr, sizeof(saddr)) == -1)
	{
		cerr << "bind error" << endl;
		return -1;
	}
	if (::listen(tcp_socket, 5) == -1)
	{
		cerr << "listen error" << endl;
		return -1;
	}
	return tcp_socket;

}


void handleAccept(int socket_fd)
{
	char buf[1024] = { '\0' };
	string cmd;
	string filename;
	recv(socket_fd, buf, sizeof(buf), 0);
	stringstream sstream;//2
	sstream << buf;
	sstream >> cmd;
	sstream >> filename;
	cout << cmd << " " << filename << endl;
	if (cmd == "GET")
	{
		ifstream file;
		filename = filename.substr(1, filename.length() - 1);
		file.open(filename, ifstream::binary);
		string head = "HTTP/1.0 200 OK\r\nContent - type:text/plain\r\n\r\n";
		if (!file)
		{
			cout << "fail" << endl;
			closesocket(socket_fd);
			return;
		}
		if (filename.find(".html") != string::npos || filename.find(".htm") != string::npos)
		{
			head = "HTTP/1.0 200 OK\r\nContent - type:text/html\r\n\r\n";
		}
		if (filename.find(".png") != string::npos)
		{
			head = "HTTP/1.0 200 OK\r\nContent - type:image/png\r\n\r\n";
		}
		if (filename.find(".jpg") != string::npos)
		{
			head = "HTTP/1.0 200 OK\r\nContent - type:image/jpg\r\n\r\n";
		}
		send(socket_fd, head.c_str(), strlen(head.c_str()), 0);
		while (!file.eof())//5
		{
			char buf[1024] = { '\0' };
			memset(buf, 0, sizeof(buf));
			file.read(buf, sizeof(buf)-1);
			int n = file.gcount();
			send(socket_fd, buf, n, 0);
		}
		file.close();
	}
	if (cmd == "table_info") {
		std::string str = "hello";
		send(socket_fd, str.c_str(), str.size(), 0);
	}
	if (cmd == "room_info") {
		std::string str = "room_info";
		send(socket_fd, str.c_str(), str.size(), 0);
	}
	closesocket(socket_fd);

}

int main(int ac, char *av[])
{
	int tcp_socket = make_server_socket(8888);
	if (tcp_socket == -1)
	{
		exit(0);
	}
	thread t;
	while (1)
	{
		int socket_fd = accept(tcp_socket, nullptr, nullptr);
		t = thread(handleAccept, socket_fd);
		t.detach();
	}

	system("pause");
}