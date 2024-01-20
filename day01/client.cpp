#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./client ip port\n");
        printf("example: ./client 172.19.163.2 9999\n");
        exit(-1);
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() failed.\n");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("connect() failed.\n");
        return -1;
    }

    printf("connect ok!\n");

    for (int i = 0; i < 200000; ++i) {
        // 从命令行输入内容
        memset(&buf, 0, sizeof(buf));
        printf("please input: "); scanf("%s", buf);

        if (send(sockfd, &buf, strlen(buf), 0) < 0) {
            printf("write() faile."); close(sockfd); return -1;
        }

        memset(&buf, 0, sizeof(buf));
        if (recv(sockfd, &buf, sizeof(buf), 0) <= 0) {     // 接收服务端的回应
            printf("read() failed."); close(sockfd); return -1;
        }

        printf("server echo: %s\n", buf);
    }
    return 0;
}