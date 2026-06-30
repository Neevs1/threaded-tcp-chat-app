#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <algorithm>

#define MAX 100

class tcp_client{
    int port, queue_length,server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buffer[MAX];
    public:
    int create_socket(){
        int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        return server_sockfd;
    }

    tcp_client(){
        
        this->server_sockfd = create_socket();
        if(server_sockfd < 0){
            std::cout<<"Encountered error while creating a socket. Terminated"<<std::endl;
            return;
        }

         // Initialize server address
        memset(&server_addr, 0, sizeof(server_addr));
        this->server_addr.sin_family = AF_INET;
        this->server_addr.sin_addr.s_addr = INADDR_ANY;
        this->server_addr.sin_port = htons(3400);

        // Bind
        int result = bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(result != 0){
            close(server_sockfd);
            std::cout<<"Invalid port number, try again\n";
            return;
        }
        int port = ntohs(server_addr.sin_port);
        std::cout<<"PORT selected is "<<port<<std::endl;
        std::cout<<"Enter length of queue"<<std::endl;
        int queue_length = 1;
        std::cin>>queue_length;
        this->queue_length =  std::max(queue_length,1);
        listenTo();

    }

    void listenTo(){
        listen(this->server_sockfd, this->queue_length );
    }

    void communicate(){
    this->addr_len = sizeof(this->client_addr);

    
    client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &addr_len);

    printf("Client connected!\n");

    while (1) {
        int n = recv(client_sockfd, buffer, MAX, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        std::cout<<"Client :"<<buffer<<std::endl;

        std::cout<<("Server reply: ");
        std::cin.getline(buffer,99);

        send(client_sockfd, buffer, strlen(buffer), 0);
    }
    }

};

int main(){
    tcp_client client;
    //client.listen();
    client.communicate();
}

