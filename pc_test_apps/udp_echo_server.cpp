#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <atomic>
#include <sched.h>
#include <fcntl.h>
#include <thread>
#include <mutex>

// Configuration
#define DEFAULT_PORT 5000
#define BUFFER_SIZE 2048

std::atomic<bool> running(true);
int sockfd = -1; // Global so we can close it to wake up the thread

void signalHandler(int signum) {
    if (running) {
        std::cout << "\n🛑 Signal received (" << signum << "). Stopping..." << std::endl;
        running = false;
        // Close socket to wake up the blocking recvfrom in the thread
        if (sockfd >= 0) {
            close(sockfd);
            sockfd = -1;
        }
    }
}

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " [port]" << std::endl;
    std::cerr << "  port: UDP port to listen on (default: " << DEFAULT_PORT << ")" << std::endl;
}

// Set process to high priority for lower latency
void optimizeThreadPriority() {
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        // Not fatal
        // std::cerr << "⚠️  Notice: Could not set SCHED_FIFO priority." << std::endl;
    } else {
        std::cout << "🚀 High priority (SCHED_FIFO) enabled for server thread!" << std::endl;
    }
}

void runServer(int port) {
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("❌ Socket creation failed");
        return;
    }

    // Optimize socket buffer sizes
    int rcvBufSize = 1024 * 1024; // 1MB
    int sndBufSize = 1024 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvBufSize, sizeof(rcvBufSize));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndBufSize, sizeof(sndBufSize));
    
#ifdef SO_BUSY_POLL
    int busyPoll = 50; // Microseconds to busy poll
    setsockopt(sockfd, SOL_SOCKET, SO_BUSY_POLL, &busyPoll, sizeof(busyPoll));
#endif

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("❌ Bind failed");
        close(sockfd);
        sockfd = -1;
        return;
    }

    // Try to set high priority for this specific thread
    optimizeThreadPriority();

    std::cout << "⚡ Ultra-Low Latency UDP Echo Server running on port " << port << std::endl;
    std::cout << "📥 Waiting for packets (Ctrl+C to stop)..." << std::endl;

    while (running) {
        socklen_t len = sizeof(clientAddr);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &len);

        if (n > 0) {
            // Processing logic:
            // "PING <timestamp> <sequence>" -> "PONG <timestamp> <sequence>"
            
            // Check for PING pattern to modify it, otherwise echo as-is
            if (n >= 4 && buffer[0] == 'P' && buffer[1] == 'I' && buffer[2] == 'N' && buffer[3] == 'G') {
                buffer[1] = 'O'; // Change 'I' to 'O' -> PONG
            }
            
            // Echo back immediately
            sendto(sockfd, buffer, n, 0, (const struct sockaddr *)&clientAddr, len);
        } else if (n < 0) {
            // If socket was closed by signal handler, break loop
            if (errno == EBADF || !running) {
                break;
            }
            // Ignore other errors (like EINTR)
        }
    }
    
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            printUsage(argv[0]);
            return 1;
        }
    }

    // Set up signal handlers BEFORE starting the thread
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Start server logic in a separate thread
    std::thread serverThread(runServer, port);

    // Wait for the server thread to finish (which happens when running=false)
    if (serverThread.joinable()) {
        serverThread.join();
    }

    std::cout << "🛑 Server stopped cleanly." << std::endl;
    return 0;
}
