#include <stdlib.h>
#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/init.h"
#include "FreeRTOS.h"
#include "task.h"

#include "http.h"

struct sockaddr_in addr;

void http_init()
{
    struct hostent *host = gethostbyname(HOST);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);
}

int sendall(int s, char *buf, size_t len)
{
    size_t total = 0;
    size_t bytesleft = len;
    size_t n;

    while (total < len)
    {
        n = send(s, buf + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }

    return n == -1 ? -1 : 0;
}

void send_http_post(char *url, char *data)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    char *request = malloc(1024);
    sprintf(request, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", url, HOST, strlen(data), data);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    sendall(sock, request, strlen(request));

    char c;
    do
    {
        recv(sock, &c, 1, 0);
    } while (c != 'H');

    close(sock);
    free(request);
}

char recv_http_get(char *url, int *width, int *height, int *freq)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    char request[256];
    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n%s", url, HOST);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    sendall(sock, request, strlen(request));

    char c;
    do
    {
        do
        {
            recv(sock, &c, 1, 0);
        } while (c != '\n');

        recv(sock, &c, 1, 0);
    } while (c != '\r');

    recv(sock, &c, 1, 0);
    recv(sock, &c, 1, 0);

    char data[256];
    recv(sock, data, 256, 0);
    sscanf(data, "\n%d\n%d\n%d", width, height, freq);

    close(sock);

    return c;
}
