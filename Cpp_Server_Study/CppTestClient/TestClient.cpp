#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

#define PORT		8888	// 예약된 포트 (FTP Port, 80 : HTTP Port)제외하여 랜덤한 포트 이용
#define PACKET_SIZE	1024	// 패킷 크기 지정

void check_host_name(int hostname) { //This function returns host name for local computer
	if (hostname == -1) {
		perror("gethostname");
		exit(1);
	}
}
void check_host_entry(struct hostent* hostentry) { //find host info from host name
	if (hostentry == NULL) {
		perror("gethostbyname");
		exit(1);
	}
}
void IP_formatter(char* IPbuffer) { //convert IP string to dotted decimal format
	if (NULL == IPbuffer) {
		perror("inet_ntoa");
		exit(1);
	}
}
char* getIP() {
	char host[256];
	char* IP;
	struct hostent* host_entry;
	int hostname;
	hostname = gethostname(host, sizeof(host)); //find the host name
	check_host_name(hostname);
	host_entry = gethostbyname(host); //find host information
	check_host_entry(host_entry);
	IP = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0])); //Convert into IP string
	printf("Current Host Name: %s\n", host);
	printf("Host IP: %s\n", IP);
	return IP;
}

int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET clientSocket;

	while (1) {
		clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET) {
			printf("Socket Error : INVALID_SOCKET");
			return -1;
		}


		SOCKADDR_IN clientAddress = {};
		ZeroMemory(&clientAddress, sizeof(clientAddress));
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_port = htons(PORT);
		clientAddress.sin_addr.s_addr = inet_addr(getIP());

		printf("Wait Connect...\n");


		int connectStatus =
			connect(clientSocket, (SOCKADDR*)&clientAddress, sizeof(clientAddress));

		if (connectStatus == SOCKET_ERROR) {
			printf("Socket Error : SOCKET_ERROR\n");
			return -1;
		}
		printf("Success Connection\n");

		char message[] = "[Client] Send Data";
		send(clientSocket, message, strlen(message), 0);

		char buffer[PACKET_SIZE] = {};
		recv(clientSocket, buffer, PACKET_SIZE, 0);


		printf("[Client:recv()] Receive Message : %s \n", buffer);
		Sleep(1000);
	}

	if (closesocket(clientSocket)) {
		printf("ERROR : closesocket\nerror code : %d", WSAGetLastError());
		exit(1);
	}

	WSACleanup();

	system("pause");
	return 0;
}