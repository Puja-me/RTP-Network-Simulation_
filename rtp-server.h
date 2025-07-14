#ifndef RTP_SERVER_H
#define RTP_SERVER_H

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <map>
#include <fstream>
#include <mutex>

struct ClientData {
    struct sockaddr_in addr;
    socklen_t addrLen;
    std::queue<std::string> packetBuffer;
    std::map<int, std::string> packetHistory;
    int packetCounter;
    std::ofstream jitterLog;
    std::string clientIP;
    int clientPort;
    
    ClientData() : packetCounter(0) {}
};

class RTPServer {
public:
    RTPServer(int port);
    ~RTPServer();

    void start(); // Starts the server
    void sendPacket(const std::string& message, struct sockaddr_in& clientAddr, socklen_t clientLen); // Sends an RTP packet
    void enableFEC(bool enable); // Enables Forward Error Correction
    void enableCongestionControl(bool enable); // Enables Congestion Control

private:
    int sockfd;
    struct sockaddr_in serverAddr;
    
    bool fecEnabled;
    bool congestionControlEnabled;
    
    std::map<std::string, ClientData> clients; // Map to store client data using IP:port as key
    std::mutex clientsMutex; // Mutex for thread-safe access to clients map
    
    std::ofstream serverLog; // File to log server-side statistics
    
    void receivePacket();
    void applyFEC(std::string& message); // FEC error correction method
    void manageCongestion(ClientData& client); // Congestion control logic
    std::string getClientKey(const struct sockaddr_in& addr); // Get unique key for client
};

#endif // RTP_SERVER_H