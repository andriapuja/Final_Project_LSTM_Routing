#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NineNodesTopology");

void RxTrace (std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interface) {
    Ptr<Packet> copy = p->Copy();
    Ipv4Header ipHeader;
    copy->RemoveHeader(ipHeader);
    std::cout << Simulator::Now().GetSeconds() << "s: Packet " << p->GetUid()
              << " | Received at Node " << context.substr(10,1)
              << " | TTL: " << (uint32_t)ipHeader.GetTtl() << std::endl;
}

void RemoveLink (Ptr<Node> node1, Ptr<Node> node2, Ptr<NetDevice> device1, Ptr<NetDevice> device2, std::string linkName) {
    Ptr<Ipv4> ipv4_1 = node1->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4_2 = node2->GetObject<Ipv4>();

    int32_t ifIndex1 = ipv4_1->GetInterfaceForDevice(device1);
    int32_t ifIndex2 = ipv4_2->GetInterfaceForDevice(device2);

    if (ifIndex1 != -1) {
        ipv4_1->SetDown(ifIndex1);
    }

    if (ifIndex2 != -1) {
        ipv4_2->SetDown(ifIndex2);
    }

    std::cout << "Link " << linkName << " is down" << std::endl;
    Ipv4GlobalRoutingHelper::RecomputeRoutingTables();
}

int main () {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(9);

    NodeContainer ncAB(nodes.Get(0), nodes.Get(1));
    NodeContainer ncAC(nodes.Get(0), nodes.Get(2));
    NodeContainer ncAD(nodes.Get(0), nodes.Get(3));
    NodeContainer ncBD(nodes.Get(1), nodes.Get(3));
    NodeContainer ncBE(nodes.Get(1), nodes.Get(4));
    NodeContainer ncCF(nodes.Get(2), nodes.Get(5));
    NodeContainer ncDG(nodes.Get(3), nodes.Get(6));
    NodeContainer ncEI(nodes.Get(4), nodes.Get(8));
    NodeContainer ncFH(nodes.Get(5), nodes.Get(7));
    NodeContainer ncGH(nodes.Get(6), nodes.Get(7));
    NodeContainer ncGI(nodes.Get(6), nodes.Get(8));

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer dAB = p2p.Install(ncAB);
    NetDeviceContainer dAC = p2p.Install(ncAC);
    NetDeviceContainer dAD = p2p.Install(ncAD);
    NetDeviceContainer dBD = p2p.Install(ncBD);
    NetDeviceContainer dBE = p2p.Install(ncBE);
    NetDeviceContainer dCF = p2p.Install(ncCF);
    NetDeviceContainer dDG = p2p.Install(ncDG);
    NetDeviceContainer dEI = p2p.Install(ncEI);
    NetDeviceContainer dFH = p2p.Install(ncFH);
    NetDeviceContainer dGH = p2p.Install(ncGH);
    NetDeviceContainer dGI = p2p.Install(ncGI);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    std::vector<Ipv4InterfaceContainer> ipInterfaces(11);

    address.SetBase("10.10.3.0", "255.255.255.0");
    ipInterfaces[0] = address.Assign(dAB);
    address.SetBase("10.10.1.0", "255.255.255.0");
    ipInterfaces[1] = address.Assign(dAC);
    address.SetBase("10.10.2.0", "255.255.255.0");
    ipInterfaces[2] = address.Assign(dAD);
    address.SetBase("10.10.4.0", "255.255.255.0");
    ipInterfaces[3] = address.Assign(dBD);
    address.SetBase("10.10.5.0", "255.255.255.0");
    ipInterfaces[4] = address.Assign(dBE);
    address.SetBase("10.10.6.0", "255.255.255.0");
    ipInterfaces[5] = address.Assign(dCF);
    address.SetBase("10.10.7.0", "255.255.255.0");
    ipInterfaces[6] = address.Assign(dDG);
    address.SetBase("10.10.8.0", "255.255.255.0");
    ipInterfaces[7] = address.Assign(dEI);
    address.SetBase("10.10.9.0", "255.255.255.0");
    ipInterfaces[8] = address.Assign(dFH);
    address.SetBase("10.10.10.0", "255.255.255.0");
    ipInterfaces[9] = address.Assign(dGH);
    address.SetBase("10.10.11.0", "255.255.255.0");
    ipInterfaces[10] = address.Assign(dGI);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t port = 9;
    UdpEchoServerHelper server(port);

    // Server on Node H
    ApplicationContainer serverAppH = server.Install(nodes.Get(7));
    serverAppH.Start(Seconds(1.0));
    serverAppH.Stop(Seconds(80.0));

    // Client on Node D
    UdpEchoClientHelper clientD(ipInterfaces[8].GetAddress(1), port);
    clientD.SetAttribute("MaxPackets", UintegerValue(100));
    clientD.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    clientD.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientAppD = clientD.Install(nodes.Get(3));
    clientAppD.Start(Seconds(2.0));
    clientAppD.Stop(Seconds(80.0));
    
    // Client on Node B
    UdpEchoClientHelper clientB(ipInterfaces[8].GetAddress(1), port);
    clientB.SetAttribute("MaxPackets", UintegerValue(100));
    clientB.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    clientB.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientAppB = clientB.Install(nodes.Get(1));
    clientAppB.Start(Seconds(2.0));
    clientAppB.Stop(Seconds(80.0));

    // Server on Node I
    ApplicationContainer serverAppI = server.Install(nodes.Get(8));
    serverAppI.Start(Seconds(1.0));
    serverAppI.Stop(Seconds(80.0));

    // Client on Node A
    UdpEchoClientHelper clientA(ipInterfaces[10].GetAddress(1), port);
    clientA.SetAttribute("MaxPackets", UintegerValue(100));
    clientA.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    clientA.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientAppA = clientA.Install(nodes.Get(0));
    clientAppA.Start(Seconds(2.0));
    clientAppA.Stop(Seconds(80.0));

    // Scheduled link removals
    Simulator::Schedule(Seconds(40.0), &RemoveLink, nodes.Get(0), nodes.Get(3), dAD.Get(0), dAD.Get(1), "A-D");
    Simulator::Schedule(Seconds(20.0), &RemoveLink, nodes.Get(3), nodes.Get(6), dDG.Get(0), dDG.Get(1), "D-G");
    Simulator::Schedule(Seconds(60.0), &RemoveLink, nodes.Get(0), nodes.Get(1), dAB.Get(0), dAB.Get(1), "A-B");

    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("dataset.tr"));

    // Packet tracing setup
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        std::ostringstream oss;
        oss << "/NodeList/" << nodes.Get(i)->GetId() << "/$ns3::Ipv4L3Protocol/Rx";
        Config::Connect(oss.str(), MakeCallback(&RxTrace));
    }

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

