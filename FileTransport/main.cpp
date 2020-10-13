#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "Sockets.hpp"
#include <string.h>
#include <stdlib.h>
#define UPLOAD "UPLD"
#define DOWNLOAD "DNLD"
#define SUCCESS "SUCCEED"
#define FAILED "FAIL"
char *readline(int fd, int maxLen) {
    char c;
    char *buff = (char *)malloc(maxLen);
    bzero(buff, maxLen);
    for (int index = 0; read(fd, &c, 1) == 1 && index < maxLen; index++) {
        if (c == '\r') {
            if (read(fd, &c, 1) == 1) {
                if (c == '\n') {
                    break;
                } else {
                    buff[index++] = '\r';
                    buff[index] = c;
                }
            } else {
                buff[index] = '\r';
                break;
            }
        }
        buff[index] = c;
    }
    return buff;
}
int upload(int sockfd, const char *source, const char *destination) {
    write(sockfd, UPLOAD, strlen(UPLOAD));
    write(sockfd, "\r\n", 2);
    write(sockfd, destination, strlen(destination));
    write(sockfd, "\r\n", 2);
    printf("%s request sent, waiting for response...\n", UPLOAD);
    int filefd = open(source, O_RDONLY);
    char *buff = (char *)malloc(1025);
    bzero(buff, 1025);
    ssize_t size;
    int index = 0;
    while((size = read(filefd, buff, 1024)) > 0) {
        printf("size: %zu\n", size);
        write(sockfd, buff, size);
        bzero(buff, 1024);
    }
    if (size < 0) {
        perror("read: ");
    }
    printf("end session\n");
    return 0;
}
int download(int sockfd, const char *source, const char *destination) {
    write(sockfd, DOWNLOAD, strlen(DOWNLOAD));
    write(sockfd, "\r\n", 2);
    write(sockfd, source, strlen(source));
    write(sockfd, "\r\n", 2);
    printf("%s request sent, waiting for response...\n", DOWNLOAD);
    int filefd = open(destination, O_CREAT | O_TRUNC | O_WRONLY);
    char *buff = (char *)malloc(1025);
    bzero(buff, 1025);
    size_t size;
    while((size = read(sockfd, buff, 1024)) > 0) {
        printf("size: %zu\n", size);
        write(filefd, buff, size);
        bzero(buff, 1024);
    }
    if (size < 0) {
        perror("read: ");
    }
    printf("end session\n");
    return 0;
}

typedef struct transportAddr {
    transportAddr(char const *command,
                  char const *sourcePath,
                  char const *destinationPath) {
        this->command           = command;
        this->sourcePath        = sourcePath;
        this->destinationPath   = destinationPath;
    }
    char const *command;
    char const *sourcePath;
    char const *destinationPath;
} transportAddr;

int main(int argc, char **argv) {
    transportAddr transAddr = transportAddr(DOWNLOAD, "/Users/arthur/Documents/Untitled.md", "/Users/arthur/Documents/UntitledTest.md");
    auto pf = [](void *captured, int sockfd){
        
        char const *command =
        ((transportAddr *)captured)->command;
        printf("command: %s\n", command);
        char const *sourcePath =
        ((transportAddr *)captured)->sourcePath;
        
        char const *destinationPath =
        ((transportAddr *)captured)->destinationPath;
        
        if (!strcmp(command, UPLOAD)) {
            upload(sockfd, sourcePath, destinationPath);
        } else if (!strcmp(command, DOWNLOAD)) {
            download(sockfd, sourcePath, destinationPath);
        }
        close(sockfd);
    };
    return client("127.0.0.1", 8001, &transAddr, pf);
}
