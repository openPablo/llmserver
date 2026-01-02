#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

constexpr char SOCKET_NAME[10] =  "llmserver";

void llmResponse(char *question) {
    for (int i = 0; question[i] != '\0'; i++) {
        question[i] = (char)toupper((unsigned char)question[i]);
    }
}

int main(void) {
    ssize_t numread;
    char buf[1024];
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(&addr.sun_path[1], SOCKET_NAME, sizeof(addr.sun_path)-2); //first byte is null which means we're using abstract socket name

    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
    }
    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
    }

    if (listen(sfd,100) == -1 ) {
        perror("listen");
    }
    int cfd;
    while (1) {
        cfd = accept(sfd, NULL, NULL);
        while ((numread = read(cfd,&buf,sizeof(buf))) > 0) {
            llmResponse(buf);
            if (write(STDOUT_FILENO,&buf,numread) != numread) {
                perror("impartial write to stdout");
            }
            if (write(cfd,&buf,numread) != numread) {
                perror("impartial write to client");
            }
        }
        if (numread == -1) {
            perror("read");
        }
        close(cfd);
    }
}
