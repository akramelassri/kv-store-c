#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "errno.h"

#include "server.h"
#include "parser.h"

#define MAX_LINE 8192

void reply(int fd, const char *s)
{
    size_t len = strlen(s);

    while (len > 0)
    {
        ssize_t n = send(fd, s, len, 0);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
            return;

        s += n;
        len -= (size_t)n;
    }
}

int server_listen(int port)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0)
    {
        perror("socket");
        return -1;
    }

    int one = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
                   &one, sizeof(one)) < 0)
    {
        perror("setsockopt");
        close(lfd);
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(lfd, (struct sockaddr *)&addr,
             sizeof(addr)) < 0)
    {
        perror("bind");
        close(lfd);
        return -1;
    }

    if (listen(lfd, 8) < 0)
    {
        perror("listen");
        close(lfd);
        return -1;
    }

    return lfd;
}

static void serve_client(int fd)
{
    char buf[MAX_LINE];
    size_t len = 0;

    for (;;)
    {
        if (len >= sizeof(buf))
        {
            reply(fd, "ERR line too long\n");
            return;
        }

        ssize_t n = recv(fd, buf + len,
                         sizeof(buf) - len, 0);

        if (n <= 0)
            return;

        len += (size_t)n;

        char *start = buf;

        for (;;)
        {
            char *nl = memchr(
                start, '\n',
                len - (size_t)(start - buf));

            if (!nl)
                break;

            *nl = '\0';

            if (handle_line(fd, start))
            {
                return;
            }
            start = nl + 1;
        }

        len -= (size_t)(start - buf);
        memmove(buf, start, len);
    }
    printf("[DEBUG]: Client disconnected or Error");
}

void server_run(int lfd)
{
    for (;;)
    {
        int cfd = accept(lfd, NULL, NULL);

        if (cfd < 0)
        {
            perror("accept");
            continue;
        }

        printf("[INFO]: Client connected\n");
        serve_client(cfd);
        close(cfd);
    }
}