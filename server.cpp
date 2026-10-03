#include <iostream>
#include <string>
#include <cstring>
#include <atomic>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 100

class TcpChatServer {
    static const int QUEUE_LENGTH = 5;

    int server_sockfd;
    struct sockaddr_in server_addr;

public:
    TcpChatServer() : server_sockfd(-1) {}

    ~TcpChatServer() {
        if (server_sockfd >= 0) close(server_sockfd);
    }

    // Returns false if setup failed
    bool setup() {
        server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sockfd < 0) {
            std::cerr << "Encountered error while creating a socket. Terminated\n";
            return false;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(3400);

        if (bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
            std::cerr << "Bind failed, try again\n";
            close(server_sockfd);
            server_sockfd = -1;
            return false;
        }

        std::cout << "PORT selected is " << ntohs(server_addr.sin_port) << std::endl;

        if (listen(server_sockfd, QUEUE_LENGTH) != 0) {
            std::cerr << "Listen failed\n";
            return false;
        }
        std::cout << "Max queue length is " << QUEUE_LENGTH << "\n";
        return true;
    }

    // Accepts clients one after another, forever. Handles one client
    // fully (in its own thread) before the loop can accept the next,
    // but the accept loop itself never stops on client disconnect.
    void run() {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);

            int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &addr_len);
            if (client_sockfd < 0) {
                std::cerr << "accept() failed\n";
                continue;
            }
            std::cout << "Client connected!\n";

            // handle this client in its own thread so the server can keep accepting
            std::thread(&TcpChatServer::communicate, this, client_sockfd).detach();
        }
    }

private:
    void communicate(int client_sockfd) {
        std::atomic<bool> running(true);
        char buffer[MAX];

        // dedicated thread for receiving from this client
        std::thread receiver([&]() {
            while (running) {
                int n = recv(client_sockfd, buffer, MAX - 1, 0);
                if (n <= 0) {
                    std::cout << "Client disconnected.\n";
                    running = false;
                    break;
                }
                buffer[n] = '\0';
                std::cout << "\nClient: " << buffer << "\nServer reply: " << std::flush;
            }
        });

        std::string line;
        while (running) {
            std::cout << "Server reply: ";
            if (!std::getline(std::cin, line)) break;
            if (line == "/quit") break;

            if (send(client_sockfd, line.c_str(), line.size(), 0) < 0) {
                std::cerr << "Send failed, connection may be closed\n";
                break;
            }
        }

        running = false;
        shutdown(client_sockfd, SHUT_RDWR);
        receiver.join();
        close(client_sockfd);
    }
};

int main() {
    TcpChatServer server;
    if (!server.setup()) {
        return 1;
    }
    server.run();
    return 0;
}