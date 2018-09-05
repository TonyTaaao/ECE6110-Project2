/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Marcos Talau <talau@users.sourceforge.net>
 *          Duy Nguyen <duy@soe.ucsc.edu>
 * Modified by:   Pasquale Imputato <p.imputato@gmail.com>
 *
 */

/**
 * These validation tests are detailed in http://icir.org/floyd/papers/redsims.ps
 *
 * In this code the tests 1, 3, 4 and 5 refer to the tests corresponding to
 * Figure 1, 3, 4, and 5 respectively from the document mentioned above.
 */

/** Network topology
 *
 *    10Mb/s, 2ms                            10Mb/s, 4ms
 * n0--------------|                    |---------------n4
 *                 |   1.5Mbps/s, 20ms  |
 *                 n2------------------n3
 *    10Mb/s, 3ms  |                    |    10Mb/s, 5ms
 * n1--------------|                    |---------------n5
 *
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RedTests");

uint32_t checkTimes;
double avgQueueSize;

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

NodeContainer n1n5;
NodeContainer n2n5;
NodeContainer n3n5;
NodeContainer n4n5;
NodeContainer n5n6;

Ipv4InterfaceContainer i1i5;
Ipv4InterfaceContainer i2i5;
Ipv4InterfaceContainer i3i5;
Ipv4InterfaceContainer i4i5;
Ipv4InterfaceContainer i5i6;

std::stringstream filePlotQueue;
std::stringstream filePlotQueueAvg;

//store time & packet number for enqueue
std::stringstream filePlotEnqueue1;
std::stringstream filePlotEnqueue2;
std::stringstream filePlotEnqueue3;
std::stringstream filePlotEnqueue4;

//store time & packet number for dequeue
std::stringstream filePlotDequeue1;
std::stringstream filePlotDequeue2;
std::stringstream filePlotDequeue3;
std::stringstream filePlotDequeue4;

//store time & packet number for packet drop
std::stringstream filePlotDrop1;
std::stringstream filePlotDrop2;
std::stringstream filePlotDrop3;
std::stringstream filePlotDrop4;


void EnqueueAtRed(Ptr<const QueueItem> item) {
  TcpHeader tcp;
  Ptr<Packet> pkt = item->GetPacket();
  pkt->PeekHeader(tcp);
  //if (tcp.GetDestinationPort()==sinkport) {count++; print xxx}

  //TODO:  Need to figure out how to print the time this packet arrived and which flow it belongs to.  Hint below in app setup.
  //REMEMBER:  The sequence number is actually in bytes not packets so to make it graph nice you'll need to manipulate to get 
  //           nice consequtive sequence numbers to graph

  //trace callback at line 457 does the same thing as this
  //Simulator::Schedule (Seconds (3), &EnqueueAtRed, item);

  if (tcp.GetDestinationPort() == 50001) {
    std::ofstream fPlotPktNumber (filePlotEnqueue1.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50002) {
    std::ofstream fPlotPktNumber (filePlotEnqueue2.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+100 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50003) {
    std::ofstream fPlotPktNumber (filePlotEnqueue3.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+200 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50004) {
    std::ofstream fPlotPktNumber (filePlotEnqueue4.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+300 << std::endl;
    fPlotPktNumber.close ();
  }

}

void DequeueAtRed(Ptr<const QueueItem> item) {
  TcpHeader tcp;
  Ptr<Packet> pkt = item->GetPacket();
  pkt->PeekHeader(tcp);

  //TODO:  Need to figure out how to print the time this packet left and which flow it belongs to.  Hint below in app setup.
  //REMEMBER:  The sequence number is actually in bytes not packets so to make it graph nice you'll need to manipulate to get 
  //           nice consequtive sequence numbers to graph

  if (tcp.GetDestinationPort() == 50001) {
    std::ofstream fPlotPktNumber (filePlotDequeue1.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50002) {
    std::ofstream fPlotPktNumber (filePlotDequeue2.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+100 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50003) {
    std::ofstream fPlotPktNumber (filePlotDequeue3.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+200 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50004) {
    std::ofstream fPlotPktNumber (filePlotDequeue4.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+300 << std::endl;
    fPlotPktNumber.close ();
  }
  
}

void DroppedAtRed(Ptr<const QueueItem> item) {
  TcpHeader tcp;
  Ptr<Packet> pkt = item->GetPacket();
  pkt->PeekHeader(tcp);

  //TODO:  Need to figure out how to print the time this packet was dropped and which flow it belongs to.  Hint below in app setup.
  //REMEMBER:  The sequence number is actually in bytes not packets so to make it graph nice you'll need to manipulate to get 
  //           nice consequtive sequence numbers to graph

  if (tcp.GetDestinationPort() == 50001) {
    std::ofstream fPlotPktNumber (filePlotDrop1.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50002) {
    std::ofstream fPlotPktNumber (filePlotDrop2.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+100 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50003) {
    std::ofstream fPlotPktNumber (filePlotDrop3.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+200 << std::endl;
    fPlotPktNumber.close ();
  }
  else if (tcp.GetDestinationPort() == 50004) {
    std::ofstream fPlotPktNumber (filePlotDrop4.str ().c_str (), std::ios::out|std::ios::app);
    fPlotPktNumber << Simulator::Now ().GetSeconds () << " " << (tcp.GetSequenceNumber().GetValue()/1000)%90+300 << std::endl;
    fPlotPktNumber.close ();
  }
}


void
CheckQueueSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = StaticCast<RedQueueDisc> (queue)->GetQueueSize ();

  avgQueueSize += qSize;
  checkTimes++;

  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueSize, queue);

  std::ofstream fPlotQueue (filePlotQueue.str ().c_str (), std::ios::out|std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();

  std::ofstream fPlotQueueAvg (filePlotQueueAvg.str ().c_str (), std::ios::out|std::ios::app);
  fPlotQueueAvg << Simulator::Now ().GetSeconds () << " " << avgQueueSize / checkTimes << std::endl;
  fPlotQueueAvg.close ();
}

void
BuildAppsTest ()
{
  // SINKs
  // #1
  uint16_t port1 = 50001;
  Address sinkLocalAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
  PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
  ApplicationContainer sinkApp1 = sinkHelper1.Install (n5n6.Get (1));
  sinkApp1.Start (Seconds (sink_start_time));
  sinkApp1.Stop (Seconds (sink_stop_time));
  // #2
  uint16_t port2 = 50002;
  Address sinkLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port2));
  PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
  ApplicationContainer sinkApp2 = sinkHelper2.Install (n5n6.Get (1));
  sinkApp2.Start (Seconds (sink_start_time));
  sinkApp2.Stop (Seconds (sink_stop_time));
  // #3
  uint16_t port3 = 50003;
  Address sinkLocalAddress3 (InetSocketAddress (Ipv4Address::GetAny (), port3));
  PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
  ApplicationContainer sinkApp3 = sinkHelper3.Install (n5n6.Get (1));
  sinkApp3.Start (Seconds (sink_start_time));
  sinkApp3.Stop (Seconds (sink_stop_time));
  // #4
  uint16_t port4 = 50004;
  Address sinkLocalAddress4 (InetSocketAddress (Ipv4Address::GetAny (), port4));
  PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
  ApplicationContainer sinkApp4 = sinkHelper4.Install (n5n6.Get (1));
  sinkApp4.Start (Seconds (sink_start_time));
  sinkApp4.Stop (Seconds (sink_stop_time));

  // Connection #1
  /*
   * Create the OnOff applications to send TCP to the server
   * onoffhelper is a client that send data to TCP destination
   */
  OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
  clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper1.SetAttribute 
    ("DataRate", DataRateValue (DataRate ("100Mb/s")));
  clientHelper1.SetAttribute 
    ("PacketSize", UintegerValue (1000));

  //this is the Application running on Node 1
  ApplicationContainer clientApps1;
  AddressValue remoteAddress1
    (InetSocketAddress (i5i6.GetAddress (1), port1));
  clientHelper1.SetAttribute ("Remote", remoteAddress1);
  clientApps1.Add (clientHelper1.Install (n1n5.Get (0)));
  clientApps1.Start (Seconds (client_start_time));
  clientApps1.Stop (Seconds (client_stop_time));

  // Connection #2
  OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
  clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper2.SetAttribute 
    ("DataRate", DataRateValue (DataRate ("100Mb/s")));
  clientHelper2.SetAttribute 
    ("PacketSize", UintegerValue (1000));

  ApplicationContainer clientApps2;
  AddressValue remoteAddress2
    (InetSocketAddress (i5i6.GetAddress (1), port2));
  clientHelper2.SetAttribute ("Remote", remoteAddress2);
  clientApps2.Add (clientHelper2.Install (n2n5.Get (0)));
  clientApps2.Start (Seconds (0.2));
  clientApps2.Stop (Seconds (client_stop_time));

  // Connection #3
  OnOffHelper clientHelper3 ("ns3::TcpSocketFactory", Address ());
  clientHelper3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper3.SetAttribute 
    ("DataRate", DataRateValue (DataRate ("100Mb/s")));
  clientHelper3.SetAttribute 
    ("PacketSize", UintegerValue (1000));

  ApplicationContainer clientApps3;
  AddressValue remoteAddress3
    (InetSocketAddress (i5i6.GetAddress (1), port3));
  clientHelper3.SetAttribute ("Remote", remoteAddress3);
  clientApps3.Add (clientHelper3.Install (n3n5.Get (0)));
  clientApps3.Start (Seconds (0.4));
  clientApps3.Stop (Seconds (client_stop_time));

  // Connection #4
  OnOffHelper clientHelper4 ("ns3::TcpSocketFactory", Address ());
  clientHelper4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper4.SetAttribute 
    ("DataRate", DataRateValue (DataRate ("100Mb/s")));
  clientHelper4.SetAttribute 
    ("PacketSize", UintegerValue (1000)); // telnet

  ApplicationContainer clientApps4;
  AddressValue remoteAddress4
    (InetSocketAddress (i5i6.GetAddress (1), port4));
  clientHelper4.SetAttribute ("Remote", remoteAddress4);
  clientApps4.Add (clientHelper4.Install (n4n5.Get (0)));
  clientApps4.Start (Seconds (0.6));
  clientApps4.Stop (Seconds (client_stop_time));
}

int
main (int argc, char *argv[])
{

  LogComponentEnable ("RedQueueDisc", LOG_LEVEL_INFO);

  std::string redLinkDataRate = "45Mbps";
  std::string redLinkDelay = "2ms";

  std::string pathOut;
  bool writeForPlot = true;
  bool writePcap = false;
  bool flowMonitor = false;

  bool printRedStats = true;

  /*
  global_start_time = 0.0;
  global_stop_time = 3.0; 
  sink_start_time = global_start_time;
  sink_stop_time = global_stop_time + 3.0;
  client_start_time = sink_start_time + 0.2;
  client_stop_time = global_stop_time - 2.0;
  */
  global_start_time = 0.0;
  global_stop_time = 3.0;
  sink_start_time = global_start_time;
  sink_stop_time = global_stop_time;
  client_start_time = sink_start_time;
  client_stop_time = global_stop_time;

  // Configuration and command line parameter parsing

  uint32_t runNumber = 70;
  // Will only save in the directory if enable opts below
  pathOut = "/Users/rantao/Desktop/6110a"; // Current directory: "."
  CommandLine cmd;
  cmd.AddValue("runNumber", "run number for random variable generation", runNumber);
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
  cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);

  cmd.Parse (argc, argv);

  RngSeedManager::SetSeed (30);
  RngSeedManager::SetRun (runNumber);

  NS_LOG_INFO ("Create nodes");
  NodeContainer c;
  //Create 6 nodes in the container
  c.Create (6);
  Names::Add ( "N1", c.Get (0));
  Names::Add ( "N2", c.Get (1));
  Names::Add ( "N3", c.Get (2));
  Names::Add ( "N4", c.Get (3));
  Names::Add ( "N5", c.Get (4));
  Names::Add ( "N6", c.Get (5));
  n1n5 = NodeContainer (c.Get (0), c.Get (4));
  n2n5 = NodeContainer (c.Get (1), c.Get (4));
  n3n5 = NodeContainer (c.Get (2), c.Get (4));
  n4n5 = NodeContainer (c.Get (3), c.Get (4));
  n5n6 = NodeContainer (c.Get (4), c.Get (5));

  //Default, don't need to do all this stuff
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  uint32_t meanPktSize = 500;

  // RED params
  NS_LOG_INFO ("Set RED params");
  //set number of queue packets
  Config::SetDefault ("ns3::RedQueueDisc::Mode", StringValue ("QUEUE_MODE_PACKETS")); //"QUEUE_MODE_BYTES": set size (in byte) of the queue
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (meanPktSize));
  Config::SetDefault ("ns3::RedQueueDisc::Wait", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (0.002));
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (5)); //5
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (30)); //15
  Config::SetDefault ("ns3::RedQueueDisc::QueueLimit", UintegerValue (1000));

  /*
  //these two lines not in red-tests.cc
  Config::SetDefault ("ns3::RedQueueDisc::LinkBandwidth", StringValue("45Mbps")); 
  Config::SetDefault ("ns3::RedQueueDisc::LinkDelay", StringValue("2ms")); 
  */


  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));

  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc ("ns3::RedQueueDisc", "LinkBandwidth", StringValue (redLinkDataRate),
                           "LinkDelay", StringValue (redLinkDelay));

  NS_LOG_INFO ("Create channels");
  PointToPointHelper p2p;

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer devn1n5 = p2p.Install (n1n5);
  tchPfifo.Install (devn1n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("4ms"));
  NetDeviceContainer devn2n5 = p2p.Install (n2n5);
  tchPfifo.Install (devn2n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("8ms"));
  NetDeviceContainer devn3n5 = p2p.Install (n3n5);
  tchPfifo.Install (devn3n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  NetDeviceContainer devn4n5 = p2p.Install (n4n5);
  tchPfifo.Install (devn4n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (redLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (redLinkDelay));
  NetDeviceContainer devn5n6 = p2p.Install (n5n6);
  // only backbone link has RED queue disc
  QueueDiscContainer queueDiscs = tchRed.Install (devn5n6);

  Ptr<QueueDisc> redQueue = queueDiscs.Get(0);

  //setup traces
  redQueue->TraceConnectWithoutContext("Enqueue", MakeCallback(&EnqueueAtRed));
  redQueue->TraceConnectWithoutContext("Dequeue", MakeCallback(&DequeueAtRed));
  redQueue->TraceConnectWithoutContext("Drop", MakeCallback(&DroppedAtRed));

  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i1i5 = ipv4.Assign (devn1n5);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i2i5 = ipv4.Assign (devn2n5);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i3i5 = ipv4.Assign (devn3n5);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i4i5 = ipv4.Assign (devn4n5);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  i5i6 = ipv4.Assign (devn5n6);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  BuildAppsTest ();

  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/red";
      ptp.EnablePcapAll (stmp.str ().c_str ());
    }

  Ptr<FlowMonitor> flowmon;
  if (flowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  if (writeForPlot)
    {
      
      filePlotQueue << pathOut << "/" << "redQueue.txt";
      filePlotQueueAvg << pathOut << "/" << "redQueueAvg.txt";

      remove (filePlotQueue.str ().c_str ());
      remove (filePlotQueueAvg.str ().c_str ());

      //Ptr<QueueDisc> queue = queueDiscs.Get (0);
      //Simulator::ScheduleNow (&CheckQueueSize, queue);
      
      Simulator::ScheduleNow(&CheckQueueSize, redQueue);
      
    }

    if (writeForPlot)
    {
      filePlotEnqueue1 << pathOut << "/" << "enqueue_node1.txt";
      filePlotEnqueue2 << pathOut << "/" << "enqueue_node2.txt";
      filePlotEnqueue3 << pathOut << "/" << "enqueue_node3.txt";
      filePlotEnqueue4 << pathOut << "/" << "enqueue_node4.txt";

      remove (filePlotEnqueue1.str ().c_str ());
      remove (filePlotEnqueue2.str ().c_str ());
      remove (filePlotEnqueue3.str ().c_str ());
      remove (filePlotEnqueue4.str ().c_str ());     
    }

    if (writeForPlot)
    {
      filePlotDequeue1 << pathOut << "/" << "dequeue_node1.txt";
      filePlotDequeue2 << pathOut << "/" << "dequeue_node2.txt";
      filePlotDequeue3 << pathOut << "/" << "dequeue_node3.txt";
      filePlotDequeue4 << pathOut << "/" << "dequeue_node4.txt";

      remove (filePlotDequeue1.str ().c_str ());
      remove (filePlotDequeue2.str ().c_str ());
      remove (filePlotDequeue3.str ().c_str ());
      remove (filePlotDequeue4.str ().c_str ());     
    }

    if (writeForPlot)
    {
      filePlotDrop1 << pathOut << "/" << "drop_node1.txt";
      filePlotDrop2 << pathOut << "/" << "drop_node2.txt";
      filePlotDrop3 << pathOut << "/" << "drop_node3.txt";
      filePlotDrop4 << pathOut << "/" << "drop_node4.txt";

      remove (filePlotDrop1.str ().c_str ());
      remove (filePlotDrop2.str ().c_str ());
      remove (filePlotDrop3.str ().c_str ());
      remove (filePlotDrop4.str ().c_str ());     
    }


  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();

  if (flowMonitor)
    {
      std::stringstream stmp;
      stmp << pathOut << "/red.flowmon";

      flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
    }

  if (printRedStats)
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

  Simulator::Destroy ();

  return 0;
}
