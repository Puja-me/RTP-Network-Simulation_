#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/rtp-server.h"
#include "ns3/rtp-client.h"

using namespace ns3;

int main() {
    // Enable logging
    LogComponentEnable("RTPServer", LOG_LEVEL_INFO);
    LogComponentEnable("RTPClient", LOG_LEVEL_INFO);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Setup network
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Install Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Setup RTP Server on Node 1
    uint16_t serverPort = 5000;
    RTPServer server(serverPort);
    server.start();

    // Setup RTP Client on Node 2
    RTPClient client(interfaces.GetAddress(0), serverPort);
    client.start();

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
