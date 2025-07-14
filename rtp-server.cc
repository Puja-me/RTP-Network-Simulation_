#include "rtp-server.h"
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

std::string RTPServer::getClientKey(const struct sockaddr_in& addr) {
    std::ostringstream oss;
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ipStr, INET_ADDRSTRLEN);
    oss << ipStr << ":" << ntohs(addr.sin_port);
    return oss.str();
}

RTPServer::RTPServer(int port) : fecEnabled(false), congestionControlEnabled(false) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Open server log file
    serverLog.open("server_stats.csv");
    if (serverLog.is_open()) {
        serverLog << "timestamp,total_clients,total_packets,avg_jitter_ms\n";
    } else {
        std::cerr << "Failed to open server log file" << std::endl;
    }
}

RTPServer::~RTPServer() {
    // Close all client log files
    for (auto& client : clients) {
        if (client.second.jitterLog.is_open()) {
            client.second.jitterLog.close();
        }
    }
    
    // Close server log file
    if (serverLog.is_open()) {
        serverLog.close();
    }
    
    close(sockfd);
}

void RTPServer::receivePacket() {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&clientAddr, &clientLen);
    if (bytesReceived < 0) {
        perror("Receive failed");
        return;
    }
    
    // Get unique client identifier and timestamp
    std::string clientKey = getClientKey(clientAddr);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long timestamp = tv.tv_sec * 1000LL + tv.tv_usec / 1000; // milliseconds
    
    // Simulate jitter with random delay (0-100ms)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> jitterDist(0, 100);
    int jitter = jitterDist(gen);
    
    std::cout << "Received from " << clientKey << ": " << buffer 
              << " (Jitter: " << jitter << "ms)" << std::endl;
    
    // Thread-safe access to clients map
    clientsMutex.lock();
    
    // If this is a new client, set up their data
    if (clients.find(clientKey) == clients.end()) {
        clients[clientKey] = ClientData();
        clients[clientKey].addr = clientAddr;
        clients[clientKey].addrLen = clientLen;
        
        // Extract client IP and port for the filename
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
        clients[clientKey].clientIP = ipStr;
        clients[clientKey].clientPort = ntohs(clientAddr.sin_port);
        
        // Create jitter log file for this client
        std::string logFilename = "jitter_" + clients[clientKey].clientIP + "_" + 
                                 std::to_string(clients[clientKey].clientPort) + ".csv";
        clients[clientKey].jitterLog.open(logFilename);
        
        if (clients[clientKey].jitterLog.is_open()) {
            clients[clientKey].jitterLog << "timestamp,packet_id,jitter_ms,delay_ms\n";
        } else {
            std::cerr << "Failed to open jitter log for client " << clientKey << std::endl;
        }
        
        std::cout << "New client connected: " << clientKey << std::endl;
    }
    
    ClientData& client = clients[clientKey];
    
    // Log jitter data for this packet
    if (client.jitterLog.is_open()) {
        client.jitterLog << timestamp << "," 
                        << client.packetCounter << "," 
                        << jitter << "," 
                        << jitter + (client.packetCounter % 5 == 0 ? 200 : 0) << "\n";
        client.jitterLog.flush(); // Ensure data is written immediately
    }
    
    // Store packet for FEC
    client.packetHistory[client.packetCounter] = buffer;
    
    // Every 4 packets, send an FEC packet
    if (fecEnabled && client.packetCounter % 4 == 0 && client.packetCounter > 0) {
        sendPacket("FEC_PACKET: " + client.packetHistory[client.packetCounter - 3], clientAddr, clientLen);
    }
    
    client.packetCounter++;
    clientsMutex.unlock();
    
    // Simulate network jitter
    std::this_thread::sleep_for(std::chrono::milliseconds(jitter));
    
    // Update server stats periodically (every 10 packets)
    static int totalPackets = 0;
    totalPackets++;
    if (totalPackets % 10 == 0 && serverLog.is_open()) {
        long long sum_jitter = 0;
        int count = 0;
        
        clientsMutex.lock();
        for (const auto& client_pair : clients) {
            count++;
        }
        clientsMutex.unlock();
        
        // Simple calculation - in reality you might want more sophisticated stats
        int avg_jitter = jitter; // This is simplified, should aggregate from all clients
        
        serverLog << timestamp << ","
                 << count << ","
                 << totalPackets << ","
                 << avg_jitter << "\n";
        serverLog.flush();
    }
    
    // Send acknowledgment
    sendPacket("Acknowledged", clientAddr, clientLen);
}

void RTPServer::sendPacket(const std::string& message, struct sockaddr_in& clientAddr, socklen_t clientLen) {
    std::string clientKey = getClientKey(clientAddr);
    
    // Simulate congestion by adding delay if packet rate is too high
    bool applyCongestionDelay = false;
    
    clientsMutex.lock();
    if (clients.find(clientKey) != clients.end()) {
        ClientData& client = clients[clientKey];
        applyCongestionDelay = (congestionControlEnabled && client.packetCounter % 5 == 0);
    }
    clientsMutex.unlock();

    if (applyCongestionDelay) {
        std::cout << "Simulated congestion for " << clientKey << "! Introducing delay..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));  // 200ms delay
    }

    sendto(sockfd, message.c_str(), message.size(), 0,
           (struct sockaddr*)&clientAddr, clientLen);
    std::cout << "Sent to " << clientKey << ": " << message << std::endl;
}

void RTPServer::start() {
    std::cout << "RTP Server started. Waiting for packets..." << std::endl;
    while (true) {
        receivePacket();
    }
}

void RTPServer::enableFEC(bool enable) {
    fecEnabled = enable;
    std::cout << "FEC " << (enable ? "enabled" : "disabled") << std::endl;
}

void RTPServer::enableCongestionControl(bool enable) {
    congestionControlEnabled = enable;
    std::cout << "Congestion Control " << (enable ? "enabled" : "disabled") << std::endl;
}

void RTPServer::manageCongestion(ClientData& client) {
    // Implement more sophisticated congestion control if needed
    // This is a placeholder for a more complex algorithm
}