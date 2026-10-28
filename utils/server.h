#ifndef SERVER_H
#define SERVER_H

int server_listen(int port);
void server_run(int lfd);
void reply(int fd, const char *s);

#endif