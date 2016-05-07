#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsdv-helper.h"
#include <iostream>
#include <cmath>

using namespace ns3;

uint16_t port = 9;

NS_LOG_COMPONENT_DEFINE ("Dsdv");

class Dsdv {
	public: 
		Dsdv();
		void Init(int, std::string, int, int, int, bool);
	private:
	  NodeContainer nodes;
		NetDeviceContainer devices;
		Ipv4InterfaceContainer interfaces;
    MobilityHelper mobility;
    int sides;
    int speed;
    int package;
    int distance;
    bool udp;
	private:
		void CreateNodes(int);
    void CreateDevices(std::string);
		void SetUpMobility(int);
		void SetUpPackageSize(int);
		void SetUpDistance(int);
    void InstallInternetStack(std::string);
    void InstallUDP();
};

int main (int argc, char **argv)
{
  Dsdv test = Dsdv();

  int sides = 5;
  int speed = 500;
  int package = 8;
  int distance = 100;
  bool udp = false;
  std::string fileName = "prueba";

  CommandLine cmd;
  cmd.AddValue ("sides", "Number of sides", sides);
  cmd.AddValue ("speed", "Mobility Speed, 0 for static", speed);
  cmd.AddValue ("package", "Package size", package);
  cmd.AddValue ("distance", "Distance between nodes", distance);
  cmd.AddValue ("udp", "Mount UDP", udp);
  cmd.Parse (argc, argv);

  test.Init(sides, fileName, speed, package, distance, udp);

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}

Dsdv::Dsdv() {

}

void Dsdv::Init(int sides, std::string fileName, int speed, int package, int distance, bool udp) {
  
  this->sides = sides;
  this->speed = speed;
  this->package = package;
  this->distance = distance;
  this->udp = udp;
  NS_LOG_UNCOND("Sides: " << sides);
  NS_LOG_UNCOND("Speed: " << speed);
  NS_LOG_UNCOND("PackageSize: " << package);
  NS_LOG_UNCOND("Distance: " << distance);
  NS_LOG_UNCOND("UDP: " << udp);

  CreateNodes(sides);
  CreateDevices(fileName);
  SetUpMobility(speed);
  SetUpPackageSize(package);
  SetUpDistance(distance);
  mobility.Install(nodes);
  InstallInternetStack(fileName);
  if (udp)
    InstallUDP();

}

void Dsdv::CreateNodes(int sides) {
  nodes.Create (sides * sides);
}

void Dsdv::CreateDevices (std::string fileName)
{

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel(wifiChannel.Create());

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream (fileName + ".tr"));
  wifiPhy.EnablePcapAll (fileName);
}

void Dsdv::SetUpMobility(int speed) {
  if (speed > 0) {
    std::stringstream speedValue;
    speedValue << "ns3::ConstantRandomVariable[Constant="
                                   << speed
                                   << "]";
     mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Speed", StringValue(speedValue.str()));
  } else {
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  }
}

void Dsdv::SetUpPackageSize(int packageSize) {
  std::stringstream sstm;
  sstm << package << "kbps";
  std::string rate (sstm.str());
  std::string phyMode ("DsssRate11Mbps");
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));
}

void Dsdv::SetUpDistance(int distance) {
  
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  for (int i = 0; i < sides; i++)
  {
    for (int j = 0; j < sides; j++)
    {
      positionAlloc ->Add(Vector( i*distance, j*distance, 0));
    }
    //positionAlloc ->Add(Vector( i*distance * 3 + (i % 2 ? 0 : distance / -2), i % 2 ? distance : i * distance / 2, 0));
  }
  
  mobility.SetPositionAllocator(positionAlloc);
}

void Dsdv::InstallInternetStack (std::string fileName)
{
  DsdvHelper dsdv;
  int updateInterval = 6;
  dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (updateInterval)));
  dsdv.Set ("SettlingTime", TimeValue (Seconds (6)));
  InternetStackHelper stack;
  stack.SetRoutingHelper (dsdv); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  //if (m_printRoutes)
    //{
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ((fileName + ".routes"), std::ios::out);
      dsdv.PrintRoutingTableAllAt (Seconds (updateInterval), routingStream);
    //}
}

void Dsdv::InstallUDP() {
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));
  Address serverAddress = Address(interfaces.GetAddress (0));
  Time interPacketInterval = Seconds (1.);
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  UdpEchoClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  for (int i = 1; i < sides * sides; i++)
  {
    apps = client.Install (nodes.Get (i));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (10.0));
  }

}
