#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define DEBUG_MODE true

#include <stdio.h>
#include <WinSock2.h>
#include <string>

#pragma comment(lib, "ws2_32")

#define PORT_NUM      33221
#define BLOG_SIZE     5
#define MAX_MSG_LEN   1024

SOCKET  sock_base[FD_SETSIZE];  // Socket을 저장할 배열
HANDLE hev_base[FD_SETSIZE];    // 이벤트 핸들을 저장할 배열
int cnt;    // 현재 배열에 보관하고있는 원소 갯수

SOCKET SetTCPServer(short pnum, int blog);  //대기 소켓 설정
void EventLoop(SOCKET sock);    //Event 처리 Loop

void AcceptProc(int index);
void ReadProc(int index);
void CloseProc(int index);

// Get My IP Address
void check_host_name(int hostname);
void check_host_entry(struct hostent* hostentry);
void IP_formatter(char* IPbuffer);
char* getIP();

int main()
{
    WSADATA wsadata;

    // Initialize Window Socket
    if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
        exit(-1);
    }

    printf("Server IP : %s\n", getIP());
    SOCKET sock = SetTCPServer(PORT_NUM, BLOG_SIZE);//대기 소켓 설정
    if (sock == -1)
    {
        perror("대기 소켓 오류");
    }
    else
    {
        EventLoop(sock);
    }
    // Finish Window Socket
    WSACleanup();
    return 0;
}

SOCKET SetTCPServer(short pnum, int blog)
{
    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//소켓 생성
    if (sock == -1)
    {
        return -1;
    }

    SOCKADDR_IN servaddr = { 0 };//소켓 주소
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT_NUM);

    int re = 0;
    //소켓 주소와 네트워크 인터페이스 결합
    re = bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (re == -1)
    {
        return -1;
    }

    re = listen(sock, blog);//백 로그 큐 설정
    if (re == -1)
    {
        return -1;
    }
    return sock;
}

HANDLE AddNetworkEvent(SOCKET sock, long net_event)
{
    HANDLE hev = WSACreateEvent();  // 네트워크 이벤트 객체 생성

    // 소켓과 네트워크 이벤트 객체를 저장
    sock_base[cnt] = sock;
    hev_base[cnt] = hev;

    // 보유 값 증가
    cnt++;

    WSAEventSelect(sock, hev, net_event);   // WSAEventSelect 모델 : 이벤트로 네트워크를 관리
    // 소켓과 네트워크 이벤트를 짝을 짓고, 처리할 이벤트를 등록한다.

    return hev;
}

void EventLoop(SOCKET sock)
{
    AddNetworkEvent(sock, FD_ACCEPT);

    while (true)
    {
        // WSAWaitForMultipleEvents : 이벤트 객체의 신호 상태를 감지
        // 이벤트를 수신할 때 까지 대기함
        int index = WSAWaitForMultipleEvents(cnt, hev_base, false, INFINITE, false);

        // WSAEnumNetworkEvents : 구체적인 네트워크 이벤트를 알아냄
        WSANETWORKEVENTS net_events;
        WSAEnumNetworkEvents(sock_base[index], hev_base[index], &net_events);

        switch (net_events.lNetworkEvents)
        {
            case FD_ACCEPT: 
                AcceptProc(index); 
                break;
            case FD_READ: 
                ReadProc(index); 
                break;
            case FD_CLOSE: 
                CloseProc(index); 
                break;
        }
    }
    closesocket(sock);//소켓 닫기
}

/*
FD_ACCEPT: 클라이언트가 접속하면 윈도우 메시지를 발생시킨다.
FD_READ: 데이터 수신이 가능하면 윈도우 메시지를 발생시킨다.
FD_WRITE: 데이터 송신이 가능하면 윈도우 메시지를 발생시킨다.
FD_CLOSE: 상대가 접속을 종료하면 윈도우 메시지를 발생시킨다.
FD_CONNECT: 접속이 완료되면 윈도우 메시지를 발생시킨다.
FD_OOB: OOB 데이터가 도착하면 윈도우 메시지를 발생시킨다.
*/

// FD_ACCEPT: 클라이언트가 접속하면 윈도우 메시지를 발생시킨다.
void AcceptProc(int index)
{
    SOCKADDR_IN cliaddr = { 0 };
    int len = sizeof(cliaddr);
    SOCKET dosock = accept(sock_base[0], (SOCKADDR*)&cliaddr, &len);

    if (cnt == FD_SETSIZE)
    {
        printf("채팅 방에 꽉 차서 %s:%d 입장못함!\n",
            inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        closesocket(dosock);
        return;
    }

    AddNetworkEvent(dosock, FD_READ | FD_CLOSE);
    printf("%s:%d 입장\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
}

// FD_READ : 데이터 수신이 가능하면 윈도우 메시지를 발생시킨다.
void ReadProc(int index)
{
    std::string msg = "";
    int msg_len = 0;

    char buf[MAX_MSG_LEN + 1] = { 0 };
    while (true) {
        msg_len = recv(sock_base[index], buf, MAX_MSG_LEN, 0);
        msg += buf;

        if (msg_len != MAX_MSG_LEN) break;
    }

    SOCKADDR_IN cliaddr = { 0 };
    int len = sizeof(cliaddr);
    getpeername(sock_base[index], (SOCKADDR*)&cliaddr, &len);
    
    std::string smsg;

    if (DEBUG_MODE) {
        smsg = "[" + std::string(inet_ntoa(cliaddr.sin_addr)) + " : " + std::to_string(ntohs(cliaddr.sin_port)) + "]";
        smsg = smsg + ":" + msg;
    }
    else {
        smsg = msg;
    }

    int len_str = smsg.length();
    for (int i = 1; i < cnt; i++)
    {
        send(sock_base[i], smsg.c_str(), len_str, 0);
    }
}

// FD_CLOSE : 상대가 접속을 종료하면 윈도우 메시지를 발생시킨다.
void CloseProc(int index)
{
    SOCKADDR_IN cliaddr = { 0 };
    int len = sizeof(cliaddr);
    getpeername(sock_base[index], (SOCKADDR*)&cliaddr, &len);
    printf("[%s:%d]  님 나감~\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    closesocket(sock_base[index]);
    WSACloseEvent(hev_base[index]);

    cnt--;
    sock_base[index] = sock_base[cnt];
    hev_base[index] = hev_base[cnt];

    std::string smsg = "[" + std::string(inet_ntoa(cliaddr.sin_addr)) + std::to_string(ntohs(cliaddr.sin_port)) + "] 님 나감~";

    for (int i = 1; i < cnt; i++)
    {
        if (i == index) continue;
        send(sock_base[i], smsg.c_str(), smsg.length(), 0);
    }
}

// GET My IP Address
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
    IP = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[2])); //Convert into IP string
    printf("Current Host Name: %s\n", host);
    printf("Host IP: %s\n", IP);
    return IP;
}