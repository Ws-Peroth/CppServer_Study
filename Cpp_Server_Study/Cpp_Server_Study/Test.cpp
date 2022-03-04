#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

#define PORT		8888	// 예약된 포트 (FTP Port, 80 : HTTP Port)제외하여 랜덤한 포트 이용
#define PACKET_SIZE	1024	// 패킷 크기 지정

int main() {
	// 소켓 초기화 정보를 저장
	WSADATA wsaData;

	// WSAStartup(WORD SocketVersion, address wsaData)
	// WORD : unsigned short
	// MAKEWORD(a, b) : 2.2라는 실수형을 WORD형식으로 만들어줌(0x0202)
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// SOCKET : Socket Handler
	SOCKET serverSocket;

	// socket(Address family specfication, Type, Protocol)

	// [ Address family specfication ]
	// PF_INET  : IPv4
	// PF_INET6 : IPv6

	// [Type]
	// SOCK_STREAM	: TCP
	// SOCK_DGRAM	: UDP

	// [Protocol]
	// IPPROTO_TCP	: TCP
	// IPPROTO_UPP	: UDP
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Socket이 생성되지 못하였을 경우
	if (serverSocket == INVALID_SOCKET) {
		printf("Socket Error : INVALID_SOCKET");
		return -1;
	}
	printf("Make Socket\n");

	// SOCKADDR_IN
	// 주소 정보의 집합

	// [sin_family]	: 주소 체계
	// AF_INET	: IPv4 Protocol
	// AF_INET6 : IPv6 Protocol
	// AF_LOCAL : Local통신을 위한 UNIX Protocol

	// [sin_port]
	// 이용할 포트 설정

	// htons(hostShort) : host to network short
	// 16bit 값을 host의 byte순서에서 network의 byte순서로 변경한다

	// [sin_addr]	: Host의 IP 주소
	// .s_addr = .S_un.S_addr (#define로 정의되어있음)
	// .s_addr	: IPv4 인터넷 주소

	// htonl(hostShort)	: host to network long
	// TCP/IP에서 사용하는 Big Endian 방식으로 통일시킴

	// [INADDR_ANY]
	// 사용가능한 임의의 LAN카드의 IP주소

	SOCKADDR_IN serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind(socket, socketAddress, size of socketAddress)
	// 소켓, 소켓 주소 정보, 주소정보의 크기를 묶어줌

	// listen(socket, backlog)
	// socket을 연결 대기상태로 만듦
	// backlog	: 들어온 요청을 처리할 Queue의 수
	// SOMAXCONN : 연결 가능한 최대  Queue크기

	bind(
		serverSocket,
		(SOCKADDR*)&serverAddress,
		sizeof(serverAddress)
	);

	printf("Wait Connect...\n");
	listen(serverSocket, SOMAXCONN);

	// clientSocket		: 수신을 받을 클라이언트 소켓
	// clientAddress	: 클라이언트 소켓의 주소 정보 집합
	// clientSize		: 클라이언트의 주소 정보 집합의 크기

	// accept(socket, address, address length)
	// 요청을 받는(server) 소켓, 소켓의 주소 정보, 주소 정보의 길이
	// 클라이언트의 접속 요청을 수락 (동기화된 함수 => 요청 대기)
	SOCKET clientSocket;
	SOCKADDR_IN clientAddress = {};
	int clientSize = sizeof(clientAddress);

	printf("Finding Client\n");
	clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientSize);
	printf("Connect Accept\n");

	if (clientSocket == INVALID_SOCKET) {
		printf("Accept Error : INVALID_SOCKET");
	}

	// buffer	: Client로부터 데이터를 수신받을 버퍼

	// recv(receiver socket, buffer, length, flage)
	// receiver socket 으로부터 buffer에 데이터를 수신 받음

	// flag : 읽을 데이터 옵션
	// MSG_OOB
	// OOB 데이터를 읽어옴
	// 긴급 데이터. SO_OOBINLINE 설정 필요

	// MSG_PEEK
	// 데이터를 읽더라도 버퍼에서 지워지지 않음. 수신된 데이터가 존재하는지 확인용

	// MSG_WAITALL
	// buffer의 크기가 다 차면 함수를 반환
	// 잘 안쓰임

	char buffer[PACKET_SIZE] = {};
	recv(clientSocket, buffer, PACKET_SIZE, 0);
	printf("[Server:recv()] Receive Message : %s \n", buffer);

	char message[] = "[Server] Send Data";
	send(clientSocket, message, strlen(message), 0);

	// closesocket(Socket)
	// 소켓을 종료시킴
	closesocket(clientSocket);
	closesocket(serverSocket);

	printf("Finished");
	// WSAStartup에서 지정한 내용을 지워준다.
	WSACleanup();
	system("pause");
	return 0;
}