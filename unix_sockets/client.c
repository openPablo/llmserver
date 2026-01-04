#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "llama.h"

constexpr char SOCKET_NAME[10] =  "llmserver";

int main(int argc, char *argv[]) {
    int sfd;
    ssize_t numread;
    char buf[1024];
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(&addr.sun_path[1], SOCKET_NAME, sizeof(addr.sun_path)-2); //first byte is null which means we're using abstract socket name

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
    }
    if (connect(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1 ) {
        perror("connect");
    }
    while ((numread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (write(sfd, buf, numread) != numread) {
            perror("write");
        }
    }
    if ((numread = read(sfd, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, numread) != numread) {
            perror("write");
        }
    }
    if (close(sfd) == -1) {
        perror("close");
    }
    return 0;
}