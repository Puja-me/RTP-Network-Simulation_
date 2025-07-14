#ifndef RTP_CLIENT_H
#define RTP_CLIENT_H

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>

class RTPClient {
public:
    RTPClient(const std::string& serverIP, int port, const std::string& clientId = "default");
    ~RTPClient();

    void start(); // Starts the client
    void sendPacket(const std::string& message); // Sends an RTP packet
    void enableFEC(bool enable); // Enables FEC on the client-side
    void stop(); // Stops the client

private:
    int sockfd;
    struct sockaddr_in serverAddr;
    bool fecEnabled;
    std::string clientId; // Client identifier
    std::ofstream jitterLog; // File to log jitter data
    std::atomic<bool> running;

    void receivePacket();
    void applyFEC(std::string& message); // FEC error correction method
    void jitterControl(); // Method to adjust for jitter
    void packetProcessingThread(); // Thread for processing received packets
};

#endif // RTP_CLIENT_H