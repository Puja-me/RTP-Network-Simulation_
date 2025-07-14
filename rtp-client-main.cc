
#include<sstream>
#include "rtp-client.h"
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <map>
#include <condition_variable>

// Global variables for client management
std::atomic<bool> running(true);
std::mutex consoleMutex;
std::mutex clientsMutex;
std::map<int, std::shared_ptr<RTPClient>> activeClients;
std::condition_variable clientCv;

// Function to send a message to a specific client
void sendMessageToClient(int clientNum, const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = activeClients.find(clientNum);
    if (it != activeClients.end()) {
        it->second->sendPacket(message);
        std::cout << "[client_" << clientNum << "] Sent: " << message << std::endl;
    } else {
        std::cout << "Client " << clientNum << " not found!" << std::endl;
    }
}

// Thread function to run a client
void runClient(const std::string& serverIP, int port, int clientNum) {
    std::string clientId = "client_" + std::to_string(clientNum);
    
    // Create the client
    std::shared_ptr<RTPClient> client = std::make_shared<RTPClient>(serverIP, port, clientId);
    client->enableFEC(true);
    
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        activeClients[clientNum] = client;
        std::cout << "[" << clientId << "] Client started and ready to send messages" << std::endl;
    }
    
    // Notify that the client is ready
    clientCv.notify_all();
    
    // Keep client running until program terminates
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Cleanup when thread exits
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        std::cout << "[" << clientId << "] Client stopping..." << std::endl;
        client->stop();
        activeClients.erase(clientNum);
    }
}

// Display help information
void displayHelp() {
    std::cout << "\n--- RTP Client Console ---" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  help                   - Show this help message" << std::endl;
    std::cout << "  list                   - List all active clients" << std::endl;
    std::cout << "  send <client> <msg>    - Send message to specific client" << std::endl;
    std::cout << "  broadcast <msg>        - Send message to all clients" << std::endl;
    std::cout << "  exit                   - Exit the program" << std::endl;
    std::cout << "-----------------------\n" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string serverIP = "127.0.0.1"; 
    int basePort = 8080;
    int numClients = 1; // Default to 1 client
    
    // Parse command line arguments
    if (argc > 1) {
        numClients = std::stoi(argv[1]);
    }
    
    if (argc > 2) {
        serverIP = argv[2];
    }
    
    std::cout << "Starting " << numClients << " clients connecting to " << serverIP << std::endl;
    
    std::vector<std::thread> clientThreads;
    
    // Create and start all clients
    for (int i = 1; i <= numClients; i++) {
        clientThreads.push_back(std::thread(runClient, serverIP, basePort, i));
        
        // Brief pause to ensure client startup is staggered
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Wait for all clients to initialize
    {
        std::unique_lock<std::mutex> lock(clientsMutex);
        clientCv.wait_for(lock, std::chrono::seconds(2), [&numClients]() {
            return activeClients.size() == static_cast<size_t>(numClients);
        });
    }
    
    std::cout << "\nAll clients initialized. Type 'help' for available commands." << std::endl;
    
    // Interactive command loop
    std::string command;
    displayHelp();
    
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, command);
        
        if (command == "exit") {
            running = false;
            std::cout << "Shutting down all clients..." << std::endl;
            break;
        }
        else if (command == "help") {
            displayHelp();
        }
        else if (command == "list") {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::cout << "Active clients:" << std::endl;
            for (const auto& pair : activeClients) {
                std::cout << "  client_" << pair.first << std::endl;
            }
        }
        else if (command.substr(0, 5) == "send ") {
            // Parse "send <client_num> <message>"
            std::istringstream iss(command.substr(5));
            int clientNum;
            std::string message;
            
            if (iss >> clientNum) {
                // Get the rest of the line as the message
                std::getline(iss, message);
                if (message.empty() || message[0] != ' ') {
                    std::cout << "Invalid format. Use: send <client_num> <message>" << std::endl;
                    continue;
                }
                message = message.substr(1); // Remove the leading space
                
                sendMessageToClient(clientNum, message);
            } else {
                std::cout << "Invalid format. Use: send <client_num> <message>" << std::endl;
            }
        }
        else if (command.substr(0, 10) == "broadcast ") {
            std::string message = command.substr(10);
            
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::cout << "Broadcasting message to all clients: " << message << std::endl;
            for (const auto& pair : activeClients) {
                pair.second->sendPacket(message);
            }
        }
        else {
            std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
        }
    }
    
    // Wait for all threads to complete
    for (auto& t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "All clients terminated." << std::endl;
    return 0;
}
