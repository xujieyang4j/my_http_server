#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

struct sockaddr_in create_socket_addr();
int accept_client(int serv_socket);
char *read_line(int sock);
void process_client_request(int client_socket);

struct sockaddr_in create_socket_addr(char *ip, int port)
{
    // 将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    // 每个字节都用0填充
    memset(&serv_addr, 0, sizeof(serv_addr));
    // 使用IPv4地址
    serv_addr.sin_family = AF_INET;
    // 具体的IP地址
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    // 端口
    serv_addr.sin_port = htons(port);
    return serv_addr;
}

int accept_client(int serv_socket)
{
    // 接受客户端请求
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    printf("listen and accept client:\n");
    return accept(serv_socket, (struct sockaddr *)&client_addr, &client_addr_size);
}

void process_client_request(int client_socket)
{
    // 解析http协议，比如：GET / HTTP/1.1
    char *http_protocol = read_line(client_socket);
    printf("http_protocol:%s\n", http_protocol);
    // 解析method
    char *method = malloc(10);
    int i = 0;
    while (1)
    {
        if (isspace(*http_protocol))
        {
            http_protocol++;
            break;
        }
        if (i > sizeof(method))
        {
            method = (char *)realloc(method, sizeof(method) + 10);
        }
        method[i] = *http_protocol;
        http_protocol++;
        i++;
    }
    // 解析path
    i = 0;
    char *path = malloc(10);
    while (1)
    {
        if (isspace(*http_protocol))
        {
            http_protocol++;
            break;
        }
        if (i > sizeof(path))
        {
            path = (char *)realloc(path, sizeof(path) + 10);
        }
        path[i] = http_protocol[i];
        http_protocol++;
        i++;
    }
    printf("method:%s\n", method);
    printf("path:%s\n", path);
    // path is /
    if (strcmp(path, "/")) {
        char *home = "my http server";
        send(client_socket, home, strlen(home), 0);
    }
}

char *read_line(int sock)
{
    char *buf = malloc(10);
    if (buf == NULL)
    {
        printf("malloc fail!");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    char c = '\0';
    int n;
    while (1)
    {
        if (c == '\n')
        {
            break;
        }
        n = recv(sock, &c, 1, 0);
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                {
                    recv(sock, &c, 1, 0);
                }
                else
                {
                    c = '\n';
                }
            }
            if (i > sizeof(buf))
            {
                // buf扩容
                buf = (char *)realloc(buf, sizeof(buf) + 10);
            }
            buf[i] = c;
            i++;
        }
        else
        {
            c = '\n';
        }
    }
    buf[i] = '\0';

    return buf;
}

int main(int argc, char const *argv[])
{
    // 创建server sokect
    int serv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // 创建sockaddr
    struct sockaddr_in serv_addr = create_socket_addr("127.0.0.1", 8080);
    // server sokect bind sockaddr
    bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    // 进入监听状态，等待用户发起请求
    listen(serv_socket, 20);

    while (1)
    {
        // 监听并接受客户请求
        int client_socket = accept_client(serv_socket);
        printf("listen and accept client success:\n");
        // 启动线程处理客户请求
        pthread_t process_client_t;
        int rs = pthread_create(&process_client_t, NULL, (void *)process_client_request, (void *)(intptr_t)client_socket);
        if (rs != 0)
        {
            close(client_socket);
            printf("create process_client pthread error!\n");
        }
    }
    close(serv_socket);
    return 0;
}
