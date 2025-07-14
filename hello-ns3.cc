#include "ns3/core-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  // Set up simulation time
  Time::SetResolution(Time::NS); // Set time resolution to nanoseconds

  // Log the message
  NS_LOG_UNCOND ("Hello, NS-3!");  // Prints "Hello, NS-3!" to the console

  // Run the simulation
  Simulator::Run ();  // Starts the simulation
  Simulator::Destroy ();  // Cleans up and destroys the simulation objects

  return 0;
}

