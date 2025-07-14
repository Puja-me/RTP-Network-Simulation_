#include "rtp-server.h"
#include <thread>

// For NS-3 simulation, include these headers
// #include "ns3/core-module.h"
// #include "ns3/network-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/point-to-point-module.h"
// #include "ns3/applications-module.h"
// #include "ns3/error-model.h"

int main(int argc, char* argv[]) {
    int port = 8080;  // Default server port
    
    // Parse command line arguments
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    std::cout << "Starting RTP Server on port " << port << std::endl;
    
    RTPServer server(port);

    // Enable features
    server.enableFEC(true);
    server.enableCongestionControl(true);

    // For NS-3 simulation
    /*
    // Set up error model in NS-3
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.05));  // 5% packet loss

    // Set up nodes and network
    NodeContainer nodes;
    nodes.Create(2);

    InternetStackHelper internet;
    internet.Install(nodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("5ms"));

    NetDeviceContainer devices = p2p.Install(nodes);
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    */

    server.start(); // Start RTP Server (this will run in the main thread)

    return 0;
}