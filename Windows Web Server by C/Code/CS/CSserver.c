#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>
#include <winsock.h>
#include <winsock2.h>
#include <ws2tcpip.h>

const int MAX_LEN = 1001000;

void PrintIP(int op,int ip) {
    char *str = op ? "Client" : "Server";
    printf("%s ip : %d.%d.%d.%d\n",str,ip&255,(ip>>8)&255,
            (ip>>16)&255,(ip>>24)&255);
}

void init_Network() {
    WSADATA wsaData;
    int nRet;
    if((nRet = WSAStartup(MAKEWORD(2,2),&wsaData)) != 0){
        printf("WSAStartup failed\n");
        exit(0);
    } 
}

int main(){
    //初始化网络
    init_Network();
    
    //创建套接字、将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    socklen_t clnt_addr_size = sizeof(clnt_addr);


    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(8000);  //端口


    PrintIP(0, serv_addr.sin_addr.s_addr);
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) == SOCKET_ERROR) {
        printf("bind error! %d\n", GetLastError());
        exit(0);
    }

    //进入监听状态，等待用户发起请求
    if (listen(serv_sock, 20) == SOCKET_ERROR) {
        printf("listen error! %d\n", GetLastError());
        exit(0);
    }
    
    char recv_buf[MAX_LEN];
    while (1) {

        //建立连接
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);//接收客户端请求
        if (clnt_sock == SOCKET_ERROR) {
            printf("Connect Failed! %d\n", GetLastError());
            exit(0);
        }
        PrintIP(1, clnt_addr.sin_addr.s_addr);

        //向客户端发送数据
        if (recv(clnt_sock, recv_buf, MAX_LEN, 0) == SOCKET_ERROR) {
            printf("ERROR! %d\n", GetLastError());
            break;
        }
        char str[] = "Server Received!";
        if (strcmp("Command : QUIT", recv_buf) == 0) {
            send(clnt_sock, "Server closed.", sizeof("Server closed.\n"),0);
            break;
        } else {
            printf("Message : %s\n", recv_buf);
            send(clnt_sock, str, sizeof(str), 0);
        }
        memset(recv_buf, 0, sizeof(recv_buf));
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}