#define HOST "lpe.sigsegv.cz"
#define PORT 80
#define URL_STATUS "/status/"
#define URL_BITMAP "/bitmap/"

void http_init();
void send_http_post(char *url, char *data);

void blink_1s();
