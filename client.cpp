#include <iostream>
#include <string>
#include <cstring>
#include <atomic>
#include <thread>
#include <limits>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 100

class TcpChatClient {
    int sockfd;
    int port;
    std::string server_ip;
    std::string username;
    std::atomic<bool> running;

public:
    TcpChatClient() : sockfd(-1), port(0), running(true) {}

    ~TcpChatClient() {
        if (sockfd >= 0) close(sockfd);
    }

    // Returns false if setup failed
    bool setup() {
        std::cout << "Enter server IP (e.g. 127.0.0.1, or a container/LAN IP)\n";
        std::cin >> server_ip;

        std::cout << "Enter port number\n";
        std::cin >> port;

        std::cout << "Enter username\n";
        std::cin >> username;
        // flush leftover newline left behind by the `cin >>` reads above
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Failed to create socket\n";
            return false;
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid IP address: " << server_ip << "\n";
            return false;
        }

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to connect to server\n";
            return false;
        }

        std::cout << "Connected to server!\n";
        return true;
    }

    void run() {
        // dedicated thread just for receiving, so we can type and receive independently
        std::thread receiver(&TcpChatClient::recvLoop, this);
        sendLoop();
        running = false;
        // shutdown() unblocks the receiver's recv() call so it can exit cleanly
        shutdown(sockfd, SHUT_RDWR);
        receiver.join();
    }

private:
    void recvLoop() {
        char buffer[MAX];
        while (running) {
            int n = recv(sockfd, buffer, MAX - 1, 0);
            if (n <= 0) {
                std::cout << "\nServer disconnected.\n";
                running = false;
                break;
            }
            buffer[n] = '\0';
            std::cout << "\nServer: " << buffer << "\n" << username << ": " << std::flush;
        }
    }

    void sendLoop() {
        std::string line;
        while (running) {
            std::cout << username << ": ";
            if (!std::getline(std::cin, line)) break;
            if (line == "/quit") break;

            if (send(sockfd, line.c_str(), line.size(), 0) < 0) {
                std::cerr << "Send failed, connection may be closed\n";
                break;
            }
        }
    }
};

int main() {
    TcpChatClient client;
    if (!client.setup()) {
        return 1;
    }
    client.run();
    return 0;
}