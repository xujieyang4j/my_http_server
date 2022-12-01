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

#define SERVER_STRING "Server: xujieyang_http_server/0.1.0\r\n"

struct sockaddr_in create_socket_addr(int);
int accept_client(int);
char *read_line(int);
void process_client_request(void *);
void headers(int);
void error_die(const char *);
int startup(u_short *);

struct sockaddr_in create_socket_addr(int port)
{
    // 将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    // 每个字节都用0填充
    memset(&serv_addr, 0, sizeof(serv_addr));
    // 使用IPv4地址
    serv_addr.sin_family = AF_INET;
    // 具体的IP地址
    // serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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

void process_client_request(void *arg)
{
    int client_socket = (intptr_t)arg;
    // 解析http协议，比如：GET / HTTP/1.1
    char *buf = read_line(client_socket);
    printf("%s", buf);
    // 解析method
    char *method = malloc(10);
    int i = 0;
    while (1)
    {
        if (isspace(*buf))
        {
            buf++;
            break;
        }
        if (i > sizeof(method))
        {
            method = (char *)realloc(method, sizeof(method) + 10);
        }
        method[i] = *buf;
        buf++;
        i++;
    }
    // 解析path
    i = 0;
    char *path = malloc(10);
    while (1)
    {
        if (isspace(*buf))
        {
            buf++;
            break;
        }
        if (i > sizeof(path))
        {
            path = (char *)realloc(path, sizeof(path) + 10);
        }
        path[i] = buf[i];
        buf++;
        i++;
    }
    // printf("method:%s\n", method);
    // printf("path:%s\n", path);
    while (1)
    {
        /* read & discard headers */ // 很重要
        char *buf2 = read_line(client_socket);
        printf("%s", buf2);
        if (*buf == '\n')
        {
            break;
        }
    }

    // path is /
    if (strcmp(path, "/") == 0)
    {
        headers(client_socket);
        char *str = "welcome xujieyang http server";
        send(client_socket, str, strlen(str), 0);
    }
    close(client_socket);
}

void headers(int client)
{
    char buf[1024];
    // (void)filename; /* could use filename to determine file type */

    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

// 解析http协议
char *read_line(int sock)
{
    char *buf = malloc(10);
    if (buf == NULL)
    {
        error_die("malloc failed");
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

void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}

int startup(u_short *port)
{
    int httpd = 0;
    int on = 1;
    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1)
    {
        error_die("socket");
    }
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        error_die("setsockopt failed");
    }
    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        error_die("bind");
    }
    if (listen(httpd, 5) < 0)
    {
        error_die("listen");
    }
    return (httpd);
}

int main(int argc, char const *argv[])
{
    // 创建server sokect
    int serv_socket = socket(PF_INET /*AF_INET*/, SOCK_STREAM, 0 /*IPPROTO_TCP*/);
    if (serv_socket == -1)
    {
        error_die("create socket failed");
    }
    // 创建sockaddr
    struct sockaddr_in serv_addr = create_socket_addr(8080);
    int on = 1;
    if ((setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        error_die("setsockopt failed");
    }
    // server sokect bind sockaddr
    if (bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error_die("bind failed");
    }
    // 进入监听状态，等待用户发起请求
    if (listen(serv_socket, 5) < 0)
    {
        error_die("listen failed");
    }
    pthread_t process_client_t;
    while (1)
    {
        // 监听并接受客户请求
        int client_socket = accept_client(serv_socket);
        printf("listen and accept client success:\n");
        // 启动线程处理客户请求
        if (pthread_create(&process_client_t, NULL, (void *)process_client_request, (void *)(intptr_t)client_socket) != 0)
        {
            perror("create process_client pthread failed");
            close(client_socket);
        }
    }
    close(serv_socket);
    return 0;
}
