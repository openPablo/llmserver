#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>

constexpr char HOSTNAME[] = "0.0.0.0";
constexpr char PORT[] = "8080";

int main(void) {
    struct addrinfo * addresses, * adress;
    struct addrinfo hints = {0};
    int lfd, cfd;
    char buf[1024];
    ssize_t numRead;

    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    int err = getaddrinfo(HOSTNAME, PORT,&hints, &addresses);
    if (err != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(err));
    }
    int optval = 1;
    for (adress = addresses; adress != NULL;adress=adress->ai_next) {
        if ((lfd = socket(adress->ai_family, adress->ai_socktype,adress->ai_protocol)) == -1) {
            continue;
        }
        if (setsockopt(lfd,SOL_SOCKET, SO_REUSEADDR,&optval, sizeof(optval)) == -1) {
            perror("setsockopt");
        }
        if (bind(lfd,adress->ai_addr,adress->ai_addrlen) == 0) {
            break;
        }
    }
    if (listen(lfd,5) == -1) {
        perror("listen");
    }
    freeaddrinfo(addresses);
    while (1) {
        if ((cfd = accept(lfd,NULL,NULL)) == -1) {
            perror("accept");
        }
        while ((numRead = read(cfd, buf, sizeof(buf))) > 0 ) {
            write(STDOUT_FILENO, buf, numRead);
            write(cfd, buf, numRead);
        }
        close(cfd);
    }
}