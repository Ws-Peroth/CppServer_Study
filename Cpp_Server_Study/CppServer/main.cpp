#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

#define PORT		8888
#define PACKET_SIZE	1024

void PrintErrorlog(std::string message, int errorCode);
void PrintLog(std::string message);

int main() {
	int respondCount = 0;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		PrintErrorlog("function WSAStartup()", WSAGetLastError());
		return -1;
	}

	SOCKET serverSocket;
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		PrintErrorlog("function socket()", WSAGetLastError());
		return -1;
	}
	PrintLog("Make Socket");

	SOCKADDR_IN serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress))) {
		PrintErrorlog("function bind()", WSAGetLastError());
		return -1;
	}

	PrintLog("Wait incoming connection...");
	if (listen(serverSocket, SOMAXCONN)) {
		PrintErrorlog("function listen()", WSAGetLastError());
		return -1;
	}

	SOCKET clientSocket;
	SOCKADDR_IN clientAddress = {};
	int clientSize = sizeof(clientAddress);

	while (1) {
		PrintLog("Finding the Client...");
		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientSize);
		PrintLog("Connection successful");

		if (clientSocket == INVALID_SOCKET) {
			PrintErrorlog("function accept()", WSAGetLastError());
			continue;
		}

		char buffer[PACKET_SIZE] = {};
		recv(clientSocket, buffer, PACKET_SIZE, 0);
		respondCount++;
		printf("[Server:recv()] Receive Message : %s (count : %d)\n", buffer, respondCount);


		char message[] = "[Server] Send Data";
		send(clientSocket, message, strlen(message), 0);


		closesocket(clientSocket);
	}

	PrintLog("Close Server");
	closesocket(serverSocket);
	WSACleanup();
	system("pause");
	return 0;
}

void PrintErrorlog(std::string message, int errorCode) {
	std::cout << "ERROR : " << message << std::endl;
	std::cout << "error code : " << errorCode << std::endl;
}

void PrintLog(std::string message) {
	std::cout << message << std::endl;
}