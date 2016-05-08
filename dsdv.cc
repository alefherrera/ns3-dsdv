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
		void Init(int, std::string, int, int, int, int, bool, int, int, int);
	private:
	  NodeContainer nodes;
		NetDeviceContainer devices;
		Ipv4InterfaceContainer interfaces;
    MobilityHelper mobility;
    int sides;
    int speed;
    int dataRate;
    int package;
    int distance;
    bool udp;
    int networkType;
    int areaSize;
    int updateInterval;
	private:
		void CreateNodes(int);
    void CreateDevices(std::string);
		void SetUpMobility(int);
		void SetUpPackageSize(int, int);
		void SetUpDistance(int);
    void InstallInternetStack(std::string, int);
    void InstallUDP();
};

int main (int argc, char **argv)
{
  Dsdv test = Dsdv();

  //valores default
  int sides = 2;
  int speed = 500;
  int dataRate = 8;
  int package = 1000;
  int distance = 100;
  bool udp = false;
  int networkType = 0;
  int areaSize = 200;
  int updateInterval = 6;
  std::string fileName = "prueba";

  CommandLine cmd;
  cmd.AddValue ("sides", "Number of sides", sides);
  cmd.AddValue ("speed", "Mobility Speed, 0 for static", speed);
  cmd.AddValue ("package", "Package size", package);
  cmd.AddValue ("dataRate", "Data Rate", dataRate);
  cmd.AddValue ("distance", "Distance between nodes", distance);
  cmd.AddValue ("udp", "Mount UDP", udp);
  cmd.AddValue ("networkType", "Type of network organization to create.", networkType);
  cmd.AddValue ("areaSize", "Size of the bounds to move the nodes.", areaSize);
  cmd.AddValue ("updateInterval", "Update inteval.", updateInterval);
  cmd.Parse (argc, argv);

  test.Init(sides, fileName, speed, package, dataRate, distance, udp, networkType, areaSize, updateInterval);

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}

Dsdv::Dsdv() {

}

void Dsdv::Init(int sides, std::string fileName, int speed, int package, int dataRate, int distance, bool udp, int networkType, int areaSize, int updateInterval) {
  
  this->sides = sides;
  this->speed = speed;
  this->dataRate = dataRate;
  this->package = package;
  this->distance = distance;
  this->udp = udp;
  this->networkType = networkType;
  this->areaSize = areaSize;
  this->updateInterval = updateInterval;

  NS_LOG_UNCOND("Executing DSDV Example with parameters:");
  NS_LOG_UNCOND("Sides: " << sides);
  NS_LOG_UNCOND("Speed: " << speed);
  NS_LOG_UNCOND("PackageSize: " << package);
  NS_LOG_UNCOND("DataRate: " << dataRate);
  NS_LOG_UNCOND("Distance: " << distance);
  NS_LOG_UNCOND("UDP: " << udp);
  NS_LOG_UNCOND("NetworkType: " << networkType);
  NS_LOG_UNCOND("AreaSize: " << areaSize);
  NS_LOG_UNCOND("UpdateInterval: " << updateInterval);

  CreateNodes(sides);
  CreateDevices(fileName);
  SetUpMobility(speed);
  SetUpPackageSize(package, dataRate);
  SetUpDistance(distance);
  mobility.Install(nodes);
  InstallInternetStack(fileName, updateInterval);
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
  wifiPhy.EnableAscii("prefix", nodes);
  wifiPhy.EnablePcapAll (fileName);
}

void Dsdv::SetUpMobility(int speed) {
  if (speed > 0) {
    std::stringstream speedValue;
    speedValue << "ns3::ConstantRandomVariable[Constant=" << speed << "]";
    mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel"
      ,"Bounds", RectangleValue (Rectangle (0, areaSize, 0, areaSize))
      ,"Speed", StringValue(speedValue.str())
      );
  } else {
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  }
}

void Dsdv::SetUpPackageSize(int packageSize, int dataRate) {
  std::stringstream sstm;
  sstm << dataRate << "kbps";
  std::string rate (sstm.str());
  std::stringstream size;
  size << packageSize;
  std::string pkgSize (size.str());
  std::string phyMode ("DsssRate11Mbps");
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue (pkgSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));
}

void Dsdv::SetUpDistance(int distance) {
  
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  switch(networkType)
  {

    // Red armada a mano
    case 0:
      positionAlloc ->Add(Vector( 50, 100, 0));
      positionAlloc ->Add(Vector( 100, 200, 0));
      positionAlloc ->Add(Vector( 100, 50, 0));
      positionAlloc ->Add(Vector( 150, 100, 0));
    break;

    // 
    case 1:
      for (int i = 0; i < sides; i++)
      {
        for (int j = 0; j < sides; j++)
        {
          positionAlloc ->Add(Vector( i*distance, j*distance, 0));
        }
      }
    break;
  }
  
  mobility.SetPositionAllocator(positionAlloc);
}

void Dsdv::InstallInternetStack (std::string fileName, int updateInterval)
{
  DsdvHelper dsdv;
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
      Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper> ((fileName + ".neighbor"), std::ios::out);
      dsdv.PrintRoutingTableAllEvery (Seconds (updateInterval), routingStream);
      dsdv.PrintNeighborCacheAllEvery (Seconds (updateInterval), neighborStream);
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
