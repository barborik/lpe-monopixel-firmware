#include <stdlib.h>
#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/init.h"

#include "http.h"

struct sockaddr_in addr;

void http_init()
{
    struct hostent *host = gethostbyname(HOST);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);
}

void send_http_post(char *url, char *data)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    char *request = malloc(256);
    sprintf(request, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", url, HOST, strlen(data), data);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    send(sock, request, strlen(request), 0);
    close(sock);

    free(request);
}
