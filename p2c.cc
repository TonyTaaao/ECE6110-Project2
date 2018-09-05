#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

// i hate typing _t so I'm making shortcuts
using i32 = int32_t;
using u32 = uint32_t;
using u16 = uint16_t;
//if you're not familiar with the above lines, it's like an oldschool typedef

//i can now do things like this:
//	i32 myInt = 5 

//instead of this: 
//	int32_t myInt = 5

// yes, i'm that lazy

NodeContainer n1nA;
NodeContainer n2nA;
NodeContainer n3nA;
NodeContainer n4nA;
NodeContainer nAnB;
NodeContainer nBn5;
NodeContainer nBn6;
NodeContainer nBn7;
NodeContainer nBn8;

NodeContainer leftLeaf;
NodeContainer rightLeaf;


Ipv4InterfaceContainer i1iA;
Ipv4InterfaceContainer i2iA;
Ipv4InterfaceContainer i3iA;
Ipv4InterfaceContainer i4iA;
Ipv4InterfaceContainer iAiB;
Ipv4InterfaceContainer iBi5;
Ipv4InterfaceContainer iBi6;
Ipv4InterfaceContainer iBi7;
Ipv4InterfaceContainer iBi8;

Ipv4InterfaceContainer leftLeafInterface;
Ipv4InterfaceContainer rightLeafInterface;

uint32_t checkTimes_A;
double avgQueueSize_A;

std::stringstream filePlotQueue_A;
std::stringstream filePlotQueueAvg_A;

uint32_t checkTimes_B;
double avgQueueSize_B;

std::stringstream filePlotQueue_B;
std::stringstream filePlotQueueAvg_B;

#define NUM 21
std::stringstream fileEnqueue[NUM];
std::stringstream fileDequeue[NUM];
std::stringstream fileDropped[NUM];

u16 port = 5000;

constexpr u32 packetSize = 1000 - 42;


//REMEMBER:  This is not an exact replica of what you need to create.  It just shows something relatively similar.
//NOTE: What it is I do in this example may not be the only way to achieve what you're trying to do for this assignment



void EnqueueAtRed(Ptr<const QueueItem> item) {
	TcpHeader tcp;
	Ptr<Packet> pkt = item->GetPacket();
	pkt->PeekHeader(tcp); //after this, can print the pkt: pkt->Print(), need to look up the print method

	//TODO:  Need to figure out how to print the time this packet arrived and which flow it belongs to.  Hint below in app setup.
	//REMEMBER:  The sequence number is actually in bytes not packets so to make it graph nice you'll need to manipulate to get 
	//           nice consequtive sequence numbers to graph

	if (tcp.GetDestinationPort() <= 5020) {
		std::ofstream fEnqueue(fileEnqueue[tcp.GetDestinationPort()-5000].str().c_str(), std::ios::out | std::ios::app);
  		fEnqueue << Simulator::Now().GetSeconds() << " " << tcp.GetSequenceNumber().GetValue()/packetSize << std::endl;
  		fEnqueue.close();
	}

}

void DequeueAtRed(Ptr<const QueueItem> item) {
	TcpHeader tcp;
	Ptr<Packet> pkt = item->GetPacket();
	pkt->PeekHeader(tcp);

	//TODO:  Need to figure out how to print the time this packet left and which flow it belongs to.  Hint below in app setup.
	//REMEMBER:  The sequence number is actually in bytes not packets so to make it graph nice you'll need to manipulate to get 
	//           nice consequtive sequence numbers to graph

	if (tcp.GetDestinationPort() <= 5020) {
		std::ofstream f(fileDequeue[tcp.GetDestinationPort()-5000].str().c_str(), std::ios::out | std::ios::app);
  		f << Simulator::Now().GetSeconds() << " " << tcp.GetSequenceNumber().GetValue()/packetSize << std::endl;
  		f.close();
	}

}

void DroppedAtRed(Ptr<const QueueItem> item) {
	TcpHeader tcp;
	Ptr<Packet> pkt = item->GetPacket();
	pkt->PeekHeader(tcp);

	//TODO:  Need to figure out how to print the time this packet was dropped and which flow it belongs to.  Hint below in app setup.
	//REMEMBER:  The sequence number is actually in bytes not packets so to make it graph nice you'll need to manipulate to get 
	//           nice consequtive sequence numbers to graph

	if (tcp.GetDestinationPort() <= 5020) {
		std::ofstream f(fileDropped[tcp.GetDestinationPort()-5000].str().c_str(), std::ios::out | std::ios::app);
  		f << Simulator::Now().GetSeconds() << " " << tcp.GetSequenceNumber().GetValue()/packetSize << std::endl;
  		f.close();
	}
}

//This code is fine for printing average and actual queue size
void CheckQueueSize_A(Ptr<QueueDisc> queue) {
  uint32_t qsize = StaticCast<RedQueueDisc>(queue)->GetQueueSize();
  avgQueueSize_A += qsize;
  checkTimes_A++;

  Simulator::Schedule(Seconds(0.01), &CheckQueueSize_A, queue);

  std::ofstream fPlotQueue(filePlotQueue_A.str().c_str(), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now().GetSeconds() << " " << qsize << std::endl;
  fPlotQueue.close();

  std::ofstream fPlotQueueAvg(filePlotQueueAvg_A.str().c_str(), std::ios::out | std::ios::app);
  fPlotQueueAvg << Simulator::Now().GetSeconds() << " " << avgQueueSize_A / checkTimes_A << std::endl;
  fPlotQueueAvg.close();
}

void CheckQueueSize_B(Ptr<QueueDisc> queue) {
  uint32_t qsize = StaticCast<RedQueueDisc>(queue)->GetQueueSize();
  avgQueueSize_B += qsize;
  checkTimes_B++;

  Simulator::Schedule(Seconds(0.01), &CheckQueueSize_B, queue);

  std::ofstream fPlotQueue(filePlotQueue_B.str().c_str(), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now().GetSeconds() << " " << qsize << std::endl;
  fPlotQueue.close();

  std::ofstream fPlotQueueAvg(filePlotQueueAvg_B.str().c_str(), std::ios::out | std::ios::app);
  fPlotQueueAvg << Simulator::Now().GetSeconds() << " " << avgQueueSize_B / checkTimes_B << std::endl;
  fPlotQueueAvg.close();
}


int main(int argc, char* argv[]) {

	std::string pathOut = "/Users/rantao/Desktop/6110c";
	bool writeForPlot = true;
	bool createVis = false;
	bool useFlowMon = false;

	//allow these to be varied via command line
	u32 runNumber = 70;
	u32 numberOfLeaves = 4;
	u32 maxPackets = 100;

	std::string leafLinkBW = "100Mbps";
	std::string leafLinkDelay = "1ms";

	std::string bottleneckLinkBW = "45Mbps";
	std::string bottleneckLinkDelay = "2ms";

	std::string animationFile = "demo.xml";


	double stopTime = 2.0;

	double minTh = 5;
	double maxTh = 15; //15->30


	CommandLine cmd;
	cmd.AddValue("runNumber", "run number for random variable generation", runNumber);
	cmd.AddValue("numberOfLeaves", "number of leaves on each side of bottleneck", numberOfLeaves);
	cmd.AddValue("animationFile", "File name for animation output", animationFile);
	cmd.AddValue("createVis", "<0/1> to create animation output", createVis);
	cmd.AddValue("writeForPlot", "<0/1> to write results for queue plot", writeForPlot);
	cmd.AddValue("useFlowMon", "<0/1> to use the flowmonitor", useFlowMon);
	cmd.AddValue("maxPackets", "Max packets allowed in the device queue", maxPackets);

	cmd.Parse(argc, argv);

	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(packetSize));
	Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));

	//RED setup
	Config::SetDefault("ns3::RedQueueDisc::Mode", StringValue("QUEUE_MODE_PACKETS"));
	Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(packetSize));
  	Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minTh));
  	Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));
  	Config::SetDefault ("ns3::RedQueueDisc::LinkBandwidth", StringValue (bottleneckLinkBW));
  	Config::SetDefault ("ns3::RedQueueDisc::LinkDelay", StringValue (bottleneckLinkDelay));
  	Config::SetDefault ("ns3::RedQueueDisc::QueueLimit", UintegerValue (maxPackets));
  	//limit the send buffer size
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue(16*packetSize));

	SeedManager::SetSeed(30); //3
	SeedManager::SetRun(runNumber);


	NodeContainer c;
	//Create 10 nodes in the container
	c.Create (10);
	Names::Add ( "N1", c.Get (0));
	Names::Add ( "N2", c.Get (1));
	Names::Add ( "N3", c.Get (2));
	Names::Add ( "N4", c.Get (3));
	Names::Add ( "N5", c.Get (4));
	Names::Add ( "N6", c.Get (5));
	Names::Add ( "N7", c.Get (6));
	Names::Add ( "N8", c.Get (7));
	Names::Add ( "N9", c.Get (8));
	Names::Add ( "N10", c.Get (9));

	n1nA = NodeContainer (c.Get (0), c.Get (4));
	n2nA = NodeContainer (c.Get (1), c.Get (4));
	n3nA = NodeContainer (c.Get (2), c.Get (4));
	n4nA = NodeContainer (c.Get (3), c.Get (4));
	nAnB = NodeContainer (c.Get (4), c.Get (5));
	nBn5 = NodeContainer (c.Get (5), c.Get (6));
	nBn6 = NodeContainer (c.Get (5), c.Get (7));
	nBn7 = NodeContainer (c.Get (5), c.Get (8));
	nBn8 = NodeContainer (c.Get (5), c.Get (9));


	//Install internet stack on all nodes.
	InternetStackHelper stack;
	stack.Install (c);

	TrafficControlHelper tchPfifo;
  	uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  	tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));

	//setup red queue on bottleneck	
	TrafficControlHelper tchBottleneck;
	tchBottleneck.SetRootQueueDisc("ns3::RedQueueDisc"); //Set for RED queue. if don't do this, will use Droptail queue(default)


	//Create channels
	PointToPointHelper p2p;

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("0.5ms"));
	NetDeviceContainer devn1nA = p2p.Install (n1nA);
	tchPfifo.Install (devn1nA);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
	NetDeviceContainer devn2nA = p2p.Install (n2nA);
	tchPfifo.Install (devn2nA);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));
	NetDeviceContainer devn3nA = p2p.Install (n3nA);
	tchPfifo.Install (devn3nA);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
	NetDeviceContainer devn4nA = p2p.Install (n4nA);
	tchPfifo.Install (devn4nA);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("0.5ms"));
	NetDeviceContainer devnBn5 = p2p.Install (nBn5);
	tchPfifo.Install (devnBn5);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
	NetDeviceContainer devnBn6 = p2p.Install (nBn6);
	tchPfifo.Install (devnBn6);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
	NetDeviceContainer devnBn7 = p2p.Install (nBn7);
	tchPfifo.Install (devnBn7);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
	NetDeviceContainer devnBn8 = p2p.Install (nBn8);
	tchPfifo.Install (devnBn8);

	p2p.SetQueue ("ns3::DropTailQueue");
	p2p.SetDeviceAttribute ("DataRate", StringValue ("45Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
	NetDeviceContainer devnAnB = p2p.Install (nAnB);
	// only backbone link has RED queue disc
	QueueDiscContainer queueDiscs = tchBottleneck.Install (devnAnB);
	Ptr<QueueDisc> redQueueA = queueDiscs.Get(0);
	Ptr<QueueDisc> redQueueB = queueDiscs.Get(1);

	//NOTE:  If I don't do the above with the RedQueueDisc, I'll end up with default queues on the bottleneck link.


	//setup traces
	redQueueA->TraceConnectWithoutContext("Enqueue", MakeCallback(&EnqueueAtRed));
	redQueueA->TraceConnectWithoutContext("Dequeue", MakeCallback(&DequeueAtRed));
	redQueueA->TraceConnectWithoutContext("Drop", MakeCallback(&DroppedAtRed));

	redQueueB->TraceConnectWithoutContext("Enqueue", MakeCallback(&EnqueueAtRed));
	redQueueB->TraceConnectWithoutContext("Dequeue", MakeCallback(&DequeueAtRed));
	redQueueB->TraceConnectWithoutContext("Drop", MakeCallback(&DroppedAtRed));


	//ASSIGN IP
	Ipv4AddressHelper ipv4;

	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	i1iA = ipv4.Assign (devn1nA);

	ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	i2iA = ipv4.Assign (devn2nA);

	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	i3iA = ipv4.Assign (devn3nA);

	ipv4.SetBase ("10.1.4.0", "255.255.255.0");
	i4iA = ipv4.Assign (devn4nA);

	ipv4.SetBase ("10.1.5.0", "255.255.255.0");
	iAiB = ipv4.Assign (devnAnB);

	ipv4.SetBase ("10.1.6.0", "255.255.255.0");
	iBi5 = ipv4.Assign (devnBn5);

	ipv4.SetBase ("10.1.7.0", "255.255.255.0");
	iBi6 = ipv4.Assign (devnBn6);

	ipv4.SetBase ("10.1.8.0", "255.255.255.0");
	iBi7 = ipv4.Assign (devnBn7);

	ipv4.SetBase ("10.1.9.0", "255.255.255.0");
	iBi8 = ipv4.Assign (devnBn8);

	//easy for for loop
	leftLeaf.Add(n1nA.Get(0));
	leftLeaf.Add(n2nA.Get(0));
	leftLeaf.Add(n3nA.Get(0));
	leftLeaf.Add(n4nA.Get(0));
	rightLeaf.Add(nBn5.Get(1));
	rightLeaf.Add(nBn6.Get(1));
	rightLeaf.Add(nBn7.Get(1));
	rightLeaf.Add(nBn8.Get(1));

	leftLeafInterface.Add(i1iA.Get(0));
	leftLeafInterface.Add(i2iA.Get(0));
	leftLeafInterface.Add(i3iA.Get(0));
	leftLeafInterface.Add(i4iA.Get(0));
	rightLeafInterface.Add(iBi5.Get(1));
	rightLeafInterface.Add(iBi6.Get(1));
	rightLeafInterface.Add(iBi7.Get(1));
	rightLeafInterface.Add(iBi8.Get(1));


	//APPLICATION

	//Configure Sources
	ApplicationContainer sources;

	ApplicationContainer sinks;

	//Install Sources
	OnOffHelper sourceHelper("ns3::TcpSocketFactory", Address());
	sourceHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	sourceHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	sourceHelper.SetAttribute("DataRate", DataRateValue(DataRate(leafLinkBW)));
	sourceHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
	//sourceHelper.SetAttribute("MaxBytes", UintegerValue(10000*packetSize));

	//NOTE:  How can you make it so you can determine which flow of traffic a packet belongs to by only 
	//       examinining the TCP header which is what is available when the traces fire?  It's something that
	//		 you set when you setup your applications

	ApplicationContainer tempSend;

	for(u32 i = 0; i < leftLeaf.GetN(); ++i) {
		//NOTE:  here I'm going to create one source on each leaf node and configure it to send to the corresponding leaf on the other side.
		//       pretend I did this... I actually did do it but I removed it to not give everything away

		sourceHelper.SetAttribute("DataRate", DataRateValue(DataRate(leafLinkBW)));
		sourceHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
		for (u32 j = 0; j < 4; j++) { //40 connections in total  //j= 5
			//build source
			AddressValue remoteAddress(InetSocketAddress (rightLeafInterface.GetAddress(i), port+5*i+j)); //get ip address of i-th leaf on the right
			sourceHelper.SetAttribute ("Remote", remoteAddress);
			sources.Add (sourceHelper.Install (leftLeaf.Get(i))); //install an onoff application on node i, and append this appliation at the end of sources

			//build sink
			Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port+5*i+j));
			PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
			sinks.Add (sinkHelper.Install (leftLeaf.Get(i))); //install on node i
		}

		//create source for 4 telnets
		AddressValue remoteAddress(InetSocketAddress (rightLeafInterface.GetAddress(i), port+5*i+4)); //get ip address of i-th leaf on the right
		sourceHelper.SetAttribute("DataRate", DataRateValue(DataRate("4000b/s"))); //telnet
		sourceHelper.SetAttribute("PacketSize", UintegerValue (400)); //telnet
		sourceHelper.SetAttribute ("Remote", remoteAddress);
		sources.Add (sourceHelper.Install (leftLeaf.Get(i)));

		Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port+5*i+4));
		PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
		sinks.Add (sinkHelper.Install (leftLeaf.Get(i)));
	}
	

	//Configure Sinks

	//NOTE:  Keep in mind I could install multiple sinks on a single node.  
	//HINT:  That will probably be useful for the first experiment if you solve the problem the way I did

	//ApplicationContainer sinks;


	//Install Sinks

	port = 5000;
	for(u32 i = 0; i < rightLeaf.GetN(); ++i) {
		//NOTE:  Here I'm going to create one sink on each leaf node on the right hand side

		sourceHelper.SetAttribute("DataRate", DataRateValue(DataRate(leafLinkBW)));
		sourceHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
		for (u32 j = 0; j < 5; j++) { //40 connections in total
			//build sink
			Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port+5*i+j));
			PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
			sinks.Add (sinkHelper.Install (rightLeaf.Get(i))); //install on node i

			//build source
			AddressValue remoteAddress(InetSocketAddress (leftLeafInterface.GetAddress(i), port+5*i+j));
			sourceHelper.SetAttribute ("Remote", remoteAddress);
			sources.Add (sourceHelper.Install (rightLeaf.Get(i)));
		}
	}

	
	//the 41th connection--telnet

	//install source
	AddressValue remoteAddress(InetSocketAddress (rightLeafInterface.GetAddress(3), 5020)); //get ip address of i-th leaf on the right
	sourceHelper.SetAttribute ("Remote", remoteAddress);
	
	sourceHelper.SetAttribute("DataRate", DataRateValue(DataRate("4000b/s")));
	sourceHelper.SetAttribute("PacketSize", UintegerValue (400)); //40 originally
	sources.Add (sourceHelper.Install (leftLeaf.Get(2)));

	//install sink
	Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), 5020));
	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
	sinks.Add (sinkHelper.Install (rightLeaf.Get(3)));
	


	sources.Start(Seconds(0));
	sinks.Start(Seconds(0));


	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	std::cout << "'Ere we go" << std::endl;

	AnimationInterface* anim = nullptr;
	FlowMonitorHelper flowMonHelper;
	Ptr<FlowMonitor> flowmon;

	if(createVis) {
		anim = new AnimationInterface(animationFile);
		anim->EnablePacketMetadata();
		anim->EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(stopTime));
	}

	if(writeForPlot) {
		filePlotQueue_A << pathOut << "/" << "redQueue_A.plot";
		filePlotQueueAvg_A << pathOut << "/" << "redQueueAvg_A.plot";

		remove(filePlotQueue_A.str().c_str());
		remove(filePlotQueueAvg_A.str().c_str());
		Simulator::ScheduleNow(&CheckQueueSize_A, redQueueA);

		filePlotQueue_B << pathOut << "/" << "redQueue_B.plot";
		filePlotQueueAvg_B << pathOut << "/" << "redQueueAvg_B.plot";

		remove(filePlotQueue_B.str().c_str());
		remove(filePlotQueueAvg_B.str().c_str());
		Simulator::ScheduleNow(&CheckQueueSize_B, redQueueB);
	}

	if(writeForPlot) {
		for (int i = 0;i < NUM; i++) {
			fileEnqueue[i] << pathOut << "/" << "Enqueue" + std::to_string(i) + ".txt";
			fileDequeue[i] << pathOut << "/" << "Dequeue" + std::to_string(i) + ".txt";
			fileDropped[i] << pathOut << "/" << "Dropped" + std::to_string(i) + ".txt";

			remove (fileEnqueue[i].str ().c_str ());
			remove (fileDequeue[i].str ().c_str ());
			remove (fileDropped[i].str ().c_str ());
		}
	}


	if(useFlowMon) {
		flowmon = flowMonHelper.InstallAll();
	}

	Simulator::Stop(Seconds(stopTime));
	Simulator::Run();

	
	if(useFlowMon) {
		std::stringstream flowOut;
		flowOut << pathOut << "/" << "red.flowmon";
		remove(flowOut.str().c_str());
		flowmon->SerializeToXmlFile(flowOut.str().c_str(), true, true);
	}

	
	u32 totalBytes = 0;

	for(u32 i = 0; i < sinks.GetN(); ++i) {
		Ptr<Application> app = sinks.Get(i);
		Ptr<PacketSink> pktSink = DynamicCast<PacketSink>(app);
		u32 recieved = pktSink->GetTotalRx();
		std::cout << "\tSink\t" << i << "\tBytes\t" << recieved << std::endl;
		totalBytes += recieved;
	}

	std::cout << std::endl << "\tTotal\t\tBytes\t" << totalBytes << std::endl;

	std::cout << "Done" << std::endl;

	if (true)
    {
      RedQueueDisc::Stats st = StaticCast<RedQueueDisc> (queueDiscs.Get (0))->GetStats ();
      std::cout << "*** RED stats from Node 5 queue ***" << std::endl;
      std::cout << "\t " << st.unforcedDrop << " drops due prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due hard mark" << std::endl;
      std::cout << "\t " << st.qLimDrop << " drops due queue full" << std::endl;

      st = StaticCast<RedQueueDisc> (queueDiscs.Get (1))->GetStats ();
      std::cout << "*** RED stats from Node 6 queue ***" << std::endl;
      std::cout << "\t " << st.unforcedDrop << " drops due prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due hard mark" << std::endl;
      std::cout << "\t " << st.qLimDrop << " drops due queue full" << std::endl;
    }

	if(anim)
		delete anim;

	Simulator::Destroy();
}

