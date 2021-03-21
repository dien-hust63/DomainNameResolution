#include "stdafx.h"
#include "stdio.h"
#include "conio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include <string>
#include <iostream>
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
bool isValidIpAddress(char *ipAddress);
void getIpFromDomain(char *buff, string &message);
void getDomainFromIp(char *buff, string &message);
int main(int argc, char * argv[]) {
	//Validate the parameters
	if (argc != 2) {
		printf("You must use : server.exe <numberPort>");
		printf("Example: server.exe 5500");
		return 0;
	}

	//Step 1: Initinate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Version is not support");
		return 0;
	}

	//Step 2: Contruct socket
	SOCKET server;
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	int serverPort;
	try
	{
		serverPort = stoi(argv[1]);
	}
	catch (const std::exception&)
	{
		printf("Invalid argument !\n");
		return 0;
	}
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("Error ! Can't bind this address");
		_getch();
		return 0;
	}
	printf("Server start !\n");

	//Step 4: Communicate with client 
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;

	while (1) {
		//receive message
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR) {
			printf("Error : %d", WSAGetLastError());
		}
		else {
			buff[ret] = '\0';
			string message = "";
			if (isValidIpAddress(buff)) {
				getDomainFromIp(buff, message);
			}
			else getIpFromDomain(buff, message);
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Receive from client %s:%d %s\n\n", clientIP, clientPort, buff);
			ret = sendto(server, message.c_str(), message.length(), 0, (sockaddr *)&clientAddr, clientAddrLen);
			if (ret == SOCKET_ERROR) {
				printf("Error: %d", WSAGetLastError());
			}
		}

	}//end while
	 //Step 5:Close socket
	closesocket(server);
	//Step 6 : Terminate Winsock
	WSACleanup();
	return 0;
}
void getDomainFromIp(char* buff, string &message) {
	struct hostent *remoteHost;
	int i = 0;
	struct in_addr addr = { 0 };
	addr.s_addr = inet_addr(buff);
	remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (remoteHost == NULL) {
		message.append("-");
		message.append("Can't get infomation.");
	}
	else {
		char* officialHostName = remoteHost->h_name;
		printf("\tOfficial name: %s\n", officialHostName);
		message.append("+");
		message.append("$");
		message.append(officialHostName);
		for (auto pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
			message.append("%");
			message.append(*pAlias);
		}
		printf("message: %s\n", message.c_str());
	}
}
void getIpFromDomain(char *buff, string &message) {
	addrinfo *result; //pointer to the linked-list 
					  //containing information about the host
	int rc;
	sockaddr_in *address;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //only focus on IPv4 address
	rc = getaddrinfo(buff, NULL, &hints, &result);
	const char breakStr[2] = "%";
	if (rc == 0) {
		struct addrinfo *tmp = NULL;
		address = (struct sockaddr_in *)result->ai_addr;
		char officialIp[128];
		inet_ntop(AF_INET, &address->sin_addr, officialIp, sizeof(officialIp));
		message.append("+");
		message.append("*");
		message.append(officialIp);
		for (tmp = result->ai_next; tmp != NULL; tmp = tmp->ai_next) {
			char ipStr[128];
			address = (struct sockaddr_in *)tmp->ai_addr;
			inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(officialIp));
			message.append(breakStr);
			message.append(ipStr);
		}
		printf("message: %s\n", message.c_str());
	}
	else {
		message.append("-");
		message.append("Can't get infomation.");
	}
	// free linked-list
	freeaddrinfo(result);
}

bool isValidIpAddress(char *ipAddress) 
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
	//return 1 when ipAddress is valid Ipv4 Address 
	return result == 1;
}