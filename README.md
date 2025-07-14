# Real Time Traffic Balancing : An intelligent approach to Network Congestion
RTP Client-Server with NS-3

## Overview
This project implements an **RTP (Real-time Transport Protocol) client-server model** using **NS-3 (Network Simulator 3)**. The system supports **Forward Error Correction (FEC)** and **Congestion Control** to enhance reliability and efficiency in real-time communication.

## Features
- **RTP Server**: Handles RTP packets, supports **FEC & Congestion Control**, and simulates network jitter & packet loss.
- **RTP Client**: Sends RTP packets, supports **FEC**, and handles jitter & delay compensation.

## File Structure
```
/ns-allinone-3.35/ns-3.35/scratch
│── rtp-server.h         # Header file for RTP server
│── rtp-server.cc        # Implementation of RTP server
│── rtp-server-main1.cc  # Main file to run RTP server
│── rtp-client.h         # Header file for RTP client
│── rtp-client.cc        # Implementation of RTP client
│── rtp-client-main.cc   # Main file to run RTP client
```

## Installation & Setup
### Prerequisites
- **NS-3** (Network Simulator 3)
- **C++ compiler** (GCC)
- **Linux-based OS** (Ubuntu recommended)

### Steps to Run
1. **Compile the files:**
   Move to the correct directory(scratch) where all above mentioned programs are present.
   ```bash
   cd ~/ns-allinone-3.35/ns-3.35/scratch
   ```
   Compile rtp-server.cc in one terminal
   ```bash
   g++ rtp-server.cc -o rtp-server
   ```
   Open another terminal and compile rtp-client.cc
   ```bash
   g++ rtp-client.cc -o rtp-client
   ```
  Alternaive:
  Move to the correct directory(scratch) where all above mentioned programs are present.
  ```bash
   cd ~/ns-allinone-3.35/ns-3.35/scratch
  ```
  Compile rtp-server.cc in one terminal
  ```bash
   g++ -std=c++11 -o rtp-server-main1 rtp-server-main1.cc rtp-server.cc \
  -I../build -I../src/core -I../src/network -I../src/internet -I../src/applications -I../src/point-to-point \
  -L../build/lib \
  -lns3.35-core-debug \
  -lns3.35-network-debug \
  -lns3.35-internet-debug \
  -lns3.35-applications-debug \
  -lns3.35-point-to-point-debug \
  -lns3.35-stats-debug \
  -lns3.35-bridge-debug \
  -lns3.35-traffic-control-debug
  ```
  Open another terminal and compile rtp-client.cc
  ```bash
  g++ -std=c++11 -o rtp-client rtp-client-main.cc rtp-client.cc \
  -I../build -I../src/core -I../src/network -I../src/internet -I../src/applications \
  -I../src/point-to-point -L../build/lib \
  -lns3.35-core-debug -lns3.35-network-debug -lns3.35-internet-debug \
  -lns3.35-applications-debug -lns3.35-point-to-point-debug \
  -lns3.35-stats-debug -lns3.35-bridge-debug -lns3.35-traffic-control-debug

   ```
  
2. **Run the Server(On one terminal):**
   ```bash
   ./rtp-server
   ```
3. **Run the Client(On another terminal,you can try running multiple client on different terminals):**
   ```bash
   ./rtp-client
   ```

## Configuration
Modify the source files to customize(if required, otherwise use the file given in this repository):
- **Server Port:** Change `int port = 8080;` in `rtp-server-main1.cc` and `rtp-client-main.cc`
- **Enable/Disable Features:**
  ```cpp
  server.enableFEC(true);
  server.enableCongestionControl(true);
  ```
  ```cpp
  client.enableFEC(true);
  ```

## References
- NS-3 Documentation: [https://www.nsnam.org/documentation/](https://www.nsnam.org/documentation/)
- Article: [Interactive RTP services with Predictable Reliability](https://github.com/Aalima201/RTP-Network-Simulation/blob/main/Interactive_RTP_services_with_predictable_reliability.pdf/)
- RTP Protocol: [RFC 3550](https://tools.ietf.org/html/rfc3550)



