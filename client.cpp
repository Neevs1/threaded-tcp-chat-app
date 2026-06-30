#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define MAX 100

int main() {
    int sockfd,port;
    char buffer[MAX];
    struct sockaddr_in server_addr;
    std::cout<<"enter port number \n";
    std::cin>>port;
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Connected to server!\n");

    while (1) {
        std::cout<<"Client: ";
        std::cin>>buffer;

        send(sockfd, buffer, strlen(buffer), 0);

        int n = recv(sockfd, buffer, MAX, 0);
        buffer[n] = '\0';

        std::cout<<"Server: "<< buffer<<'\n';
    }

    close(sockfd);

    return 0;
}
/*
Output :
onnected to server!
Client: Hello
Server: Hello
Client: 

*/