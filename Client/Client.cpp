#include "stdafx.h"
#include "stdio.h"
#include "conio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include <string>
#include <iostream>
#include <vector>
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

vector<string> delimiterString(string s);
void printVect(vector<string> vect1);
int main(int argc, char* argv[]) {
	//Validate the parameters
	if (argc != 3) {
		printf("You must use : client.exe <serverIp> <serverPort>\n");
		printf("Example: client.exe 127.0.0.1 5500");
		return 0;
	}

	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Version is not support.\n");
		return 0;
	}

	//Step 2: Construct Socket
	SOCKET client;
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("Client start!\n");

	// Set Time-out for receiving
	int tv = 10000; // time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(int));

	//Step 3:Specify server address
	sockaddr_in serverAddr;
	u_short serverPort = (u_short)strtoul(argv[2], NULL, 0);//contains server Port get from parameters
	char *serverIp = argv[1]; //contains server IP address get from parameters
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

	//Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr);
	while (1) {
		//send to server
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		if (strlen(buff) == 0) return 0;//exit when input is empty 
		ret = sendto(client, buff, strlen(buff), 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error! Cannot send message.\n");
		//Receive from server
		ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Can't not receive infomation");
		}
		else {
			buff[ret] = '\0';
			if (strlen(buff) == 0) printf("Not found infomation.\n");
			vector<string> vect1 = delimiterString(buff);
			string offcialElement = vect1[0].substr(2, vect1[0].length() - 1); 
			if (buff[0] == '+') { //success message
				if (buff[1] == '$') { // host name message
					cout << "Official name: " << offcialElement << endl;;
					cout << "Alias name: " << endl;
					printVect(vect1);
				}
				if (buff[1] == '*') {//Ip address message
					cout << "Official Ip: " << offcialElement <<endl;
					cout << "Alias Ip:" << endl;
					printVect(vect1);
				}
			}
			else if (buff[0] == '-') { //fail message
				printf("Not found infomation.\n");
			}
		}
	}

	//Step 5: close socket
	closesocket(client);

	//Step 6: Terminate Winsock
	WSACleanup();
	return 0;
}

vector<string> delimiterString(string s) { //split string using delimiter 
	vector<string> vect1;
	string token;
	string delimiter = "%";
	size_t pos = 0;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		vect1.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	vect1.push_back(s);
	return vect1;
}
void printVect(vector<string> vect1) {
	for (unsigned int i = 1; i < vect1.size(); ++i) {
		cout << vect1[i] << endl;
	}
}