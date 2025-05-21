#define HOST "lpe.sigsegv.cz"
#define PORT 80
#define URL_STATUS "/status/"
#define URL_BITMAP "/bitmap/"
#define URL_SHOOT  "/shoot/"

void http_init();
void send_http_post(char *url, char *data);
char recv_http_get(char *url, int *width, int *height, int *freq);

