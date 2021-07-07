#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#define MAX_LEN 1001000
#define NAME_LEN 101
//打印IP地址和端口号
void PrintIP(int op,int ip, int port) {
    char *str = op ? "Client" : "Server";
    printf("%s ip : %d.%d.%d.%d, port = %d\n",str,ip&255,(ip>>8)&255,
            (ip>>16)&255,(ip>>24)&255, port);
}
//初始化网络
void init_Network() {
    WSADATA wsaData;
    int nRet;
    if((nRet = WSAStartup(MAKEWORD(2,2),&wsaData)) != 0){
        printf("WSAStartup failed\n");
        exit(0);
    }
}
//从http报文中获取文件名
void* GetFileName(char *name, const char *s) {
    char *p = strstr(s, "GET ");
    if (p == NULL) return NULL;
    int cnt = 0;
    memset(name, 0, NAME_LEN);
    for (char *i = p + 5; *i != ' '; ++i){
        //if (*i == '\\' || *i == '/')continue;
        name[cnt++] = *i;
    }
}

//得到去掉结尾换行符的"纯净"字符串
char* fcgets(char *data, int n, FILE* __restrict__ file) {
    char *res = fgets(data, n, file);
    char *find = strchr(data, '\n');
    if (find) *find = '\0';
    return res;
}

//读取配置信息
void Read_configurations(const char *cfilename, int *WEBPORT, char *root) {
    const int PATH_LENGTH = 1001;
    FILE *f = fopen(cfilename, "r");
    const int MML = 100;
    char line[100];
    while (fcgets(line, MML, f)) {
        char* p = strstr(line, "root=");
        if (p != NULL){
            p = strstr(line, "=");
            memcpy(root, p + 1, PATH_LENGTH);
        }

        p = strstr(line, "port=");
        if (p != NULL) {
            p = strstr(line, "=");
            int x = 0;
            for (char *i = p + 1; *i != '\0'; ++i) {
                x = 10 * x + (*i) - '0';
            }
            *WEBPORT = x;
        }
    }
}
//服务器返回的文件内容
char file_contents[MAX_LEN];

void GetContent(FILE *f) {
    char c;
    memset(file_contents, 0, sizeof(file_contents));
    int cur = 0;
    while ((c = fgetc(f)) != EOF) {
        file_contents[cur++] = c;
    }
}
//服务器向客户端发送文件(以字符方式)
int sendHTML(char header[], const char *filename, int clnt_sock) {
    FILE* f = fopen(filename, "r");
    if (f == NULL)return -1;
    GetContent(f);
    fclose(f);
    int sz = strlen(file_contents);
    sprintf(header, "HTTP/1.1 200 OK\nConnection: close\nContent-Type: text/html;charset=utf-8\nContent-Length: %d\n\n", sz);
    const char *tmp = file_contents;
    return ~send(clnt_sock, header, strlen(header), 0) && ~send(clnt_sock, tmp, sz, 0);
}
//保存日志文件
void service_logs(char *s) {
    FILE *fp = fopen("service.log", "a");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fprintf(fp, "%s", s);
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
}

int main(){
    //初始化网络
    init_Network();
    
    //创建套接字
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    //读取配置信息
    int WEBPORT;
    char root[1001];
    Read_configurations("www.ini", &WEBPORT, root);

    //创建socket对象
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(WEBPORT);  //端口
    
    //输出服务器地址, 端口
    PrintIP(0, serv_addr.sin_addr.s_addr, WEBPORT);

    //将套接字和IP、端口绑定
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
        PrintIP(1, clnt_addr.sin_addr.s_addr, WEBPORT);

        //获取客户端请求的文件, http 请求报文保存在recv_buf中
        if (recv(clnt_sock, recv_buf, MAX_LEN, 0) == SOCKET_ERROR) {
            printf("ERROR! %d\n", GetLastError());
            break;
        }
        
        //获取日志
        service_logs(recv_buf);

        //获取文件名
        char name[1001];
        GetFileName(name, recv_buf);
        printf("%s\n",name);

        //向客户端发送HTML文件
        char header[MAX_LEN];
        if(sendHTML(header, name, clnt_sock));
        else {
            sprintf(header, "HTTP/1.1 404 Not Found");
            send(clnt_sock, header, sizeof(header), 0);
        }
        //向客户端发送数据
        /*char str[] = "Server Received!";
        if (strcmp("Command : QUIT", recv_buf) == 0) {
            send(clnt_sock, "Server closed.", sizeof("Server closed.\n"),0);
            break;
        } else {
            //printf("Message : %s\n", recv_buf);
            send(clnt_sock, str, sizeof(str), 0);
        }
        memset(recv_buf, 0, sizeof(recv_buf));
        */
        close(clnt_sock);
    }

    //关闭服务器
    close(serv_sock);
    return 0;
}