#include "rtp-client.h"
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <queue>
#include <chrono>
#include <mutex>
#include <sys/time.h>

std::queue<std::string> jitterBuffer;  // Buffer to handle delayed packets
std::mutex jitterBufferMutex;

RTPClient::RTPClient(const std::string& serverIP, int port, const std::string& clientId)
    : fecEnabled(false), clientId(clientId), running(true) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        perror("Invalid server address");
        exit(EXIT_FAILURE);
    }
    
    // Create jitter log file
    std::string logFilename = "client_jitter_" + clientId + ".csv";
    jitterLog.open(logFilename);
    
    if (jitterLog.is_open()) {
        jitterLog << "timestamp,packet_id,buffer_size,processing_time_ms\n";
    } else {
        std::cerr << "Failed to open jitter log file" << std::endl;
    }
}

RTPClient::~RTPClient() {
    if (jitterLog.is_open()) {
        jitterLog.close();
    }
    close(sockfd);
}

void RTPClient::sendPacket(const std::string& message) {
    static int packetId = 0;
    packetId++;
    
    // Get current timestamp
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long sendTimestamp = tv.tv_sec * 1000LL + tv.tv_usec / 1000; // milliseconds
    
    // Add packet ID to message for tracking
    std::string packetMessage = "[" + clientId + "|" + std::to_string(packetId) + "] " + message;
    
    sendto(sockfd, packetMessage.c_str(), packetMessage.size(), 0,
           (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "[" << clientId << "] Sent: " << message << " (ID: " << packetId << ")" << std::endl;

    // We don't block here for receiving - that's handled by the separate thread
}

void RTPClient::packetProcessingThread() {
    char buffer[1024];
    int packetId = 0;
    
    while (running) {
        socklen_t len = sizeof(serverAddr);
        int bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                    (struct sockaddr*)&serverAddr, &len);
                                    
        if (bytesReceived > 0 && running) {
            buffer[bytesReceived] = '\0';
            
            // Get receive timestamp
            struct timeval tv;
            gettimeofday(&tv, NULL);
            long long receiveTimestamp = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
            
            // Add to jitter buffer
            jitterBufferMutex.lock();
            jitterBuffer.push(std::string(buffer));
            int bufferSize = jitterBuffer.size();
            jitterBufferMutex.unlock();
            
            // Simulate processing time (proportional to buffer size)
            int processingTime = 10 + (bufferSize * 5); // Base 10ms + 5ms per buffered packet
            std::this_thread::sleep_for(std::chrono::milliseconds(processingTime));
            
            // Log jitter data
            if (jitterLog.is_open()) {
                jitterLog << receiveTimestamp << ","
                         << ++packetId << ","
                         << bufferSize << ","
                         << processingTime << "\n";
                jitterLog.flush();
            }
            
            // Process a packet from the buffer if it has enough packets
            jitterBufferMutex.lock();
            if (jitterBuffer.size() > 3) {  // Simulating processing delay
                std::cout << "[" << clientId << "] Processed from buffer: " << jitterBuffer.front() << std::endl;
                jitterBuffer.pop();
            }
            jitterBufferMutex.unlock();
        }
    }
}

void RTPClient::start() {
    // Start the packet processing thread
    std::thread processingThread(&RTPClient::packetProcessingThread, this);
    processingThread.detach(); // Detach so it runs independently
    
    std::string message;
    while (running) {
        std::cout << "[" << clientId << "] Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);
        
        if (message == "exit") {
            running = false;
            break;
        }
        
        sendPacket(message);
    }
}

void RTPClient::stop() {
    running = false;
    // Send a dummy packet to unblock the recvfrom in the processing thread
    sendto(sockfd, "EXIT", 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}

void RTPClient::enableFEC(bool enable) {
    fecEnabled = enable;
    std::cout << "[" << clientId << "] FEC Enabled: " << (enable ? "Yes" : "No") << std::endl;
}