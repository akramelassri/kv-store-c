#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int port = 7331;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    listen(lfd, 8);

    printf("listening on %d\n", port);

    char buf[4096];

    for (;;) {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0) {
            perror("accept");
            continue;
        }
        printf("Connected Client\n");

        ssize_t n;
        while ((n = recv(cfd, buf, sizeof buf, 0)) > 0) {
            send(cfd, buf, n, 0); // just echo back
        }
        close(cfd);
        printf("Closed Connection\n");
    }
}
