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

SOCKET  sock_base[FD_SETSIZE];  // Socket�� ������ �迭
HANDLE hev_base[FD_SETSIZE];    // �̺�Ʈ �ڵ��� ������ �迭
int cnt;    // ���� �迭�� �����ϰ��ִ� ���� ����

SOCKET SetTCPServer(short pnum, int blog);  //��� ���� ����
void EventLoop(SOCKET sock);    //Event ó�� Loop

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
    SOCKET sock = SetTCPServer(PORT_NUM, BLOG_SIZE);//��� ���� ����
    if (sock == -1)
    {
        perror("��� ���� ����");
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
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//���� ����
    if (sock == -1)
    {
        return -1;
    }

    SOCKADDR_IN servaddr = { 0 };//���� �ּ�
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT_NUM);

    int re = 0;
    //���� �ּҿ� ��Ʈ��ũ �������̽� ����
    re = bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (re == -1)
    {
        return -1;
    }

    re = listen(sock, blog);//�� �α� ť ����
    if (re == -1)
    {
        return -1;
    }
    return sock;
}

HANDLE AddNetworkEvent(SOCKET sock, long net_event)
{
    HANDLE hev = WSACreateEvent();  // ��Ʈ��ũ �̺�Ʈ ��ü ����

    // ���ϰ� ��Ʈ��ũ �̺�Ʈ ��ü�� ����
    sock_base[cnt] = sock;
    hev_base[cnt] = hev;

    // ���� �� ����
    cnt++;

    WSAEventSelect(sock, hev, net_event);   // WSAEventSelect �� : �̺�Ʈ�� ��Ʈ��ũ�� ����
    // ���ϰ� ��Ʈ��ũ �̺�Ʈ�� ¦�� ����, ó���� �̺�Ʈ�� ����Ѵ�.

    return hev;
}

void EventLoop(SOCKET sock)
{
    AddNetworkEvent(sock, FD_ACCEPT);

    while (true)
    {
        // WSAWaitForMultipleEvents : �̺�Ʈ ��ü�� ��ȣ ���¸� ����
        // �̺�Ʈ�� ������ �� ���� �����
        int index = WSAWaitForMultipleEvents(cnt, hev_base, false, INFINITE, false);

        // WSAEnumNetworkEvents : ��ü���� ��Ʈ��ũ �̺�Ʈ�� �˾Ƴ�
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
    closesocket(sock);//���� �ݱ�
}

/*
FD_ACCEPT: Ŭ���̾�Ʈ�� �����ϸ� ������ �޽����� �߻���Ų��.
FD_READ: ������ ������ �����ϸ� ������ �޽����� �߻���Ų��.
FD_WRITE: ������ �۽��� �����ϸ� ������ �޽����� �߻���Ų��.
FD_CLOSE: ��밡 ������ �����ϸ� ������ �޽����� �߻���Ų��.
FD_CONNECT: ������ �Ϸ�Ǹ� ������ �޽����� �߻���Ų��.
FD_OOB: OOB �����Ͱ� �����ϸ� ������ �޽����� �߻���Ų��.
*/

// FD_ACCEPT: Ŭ���̾�Ʈ�� �����ϸ� ������ �޽����� �߻���Ų��.
void AcceptProc(int index)
{
    SOCKADDR_IN cliaddr = { 0 };
    int len = sizeof(cliaddr);
    SOCKET dosock = accept(sock_base[0], (SOCKADDR*)&cliaddr, &len);

    if (cnt == FD_SETSIZE)
    {
        printf("ä�� �濡 �� ���� %s:%d �������!\n",
            inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        closesocket(dosock);
        return;
    }

    AddNetworkEvent(dosock, FD_READ | FD_CLOSE);
    printf("%s:%d ����\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
}

// FD_READ : ������ ������ �����ϸ� ������ �޽����� �߻���Ų��.
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

// FD_CLOSE : ��밡 ������ �����ϸ� ������ �޽����� �߻���Ų��.
void CloseProc(int index)
{
    SOCKADDR_IN cliaddr = { 0 };
    int len = sizeof(cliaddr);
    getpeername(sock_base[index], (SOCKADDR*)&cliaddr, &len);
    printf("[%s:%d]  �� ����~\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    closesocket(sock_base[index]);
    WSACloseEvent(hev_base[index]);

    cnt--;
    sock_base[index] = sock_base[cnt];
    hev_base[index] = hev_base[cnt];

    std::string smsg = "[" + std::string(inet_ntoa(cliaddr.sin_addr)) + std::to_string(ntohs(cliaddr.sin_port)) + "] �� ����~";

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