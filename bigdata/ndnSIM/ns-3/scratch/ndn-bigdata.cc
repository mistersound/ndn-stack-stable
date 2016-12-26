/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/node-container.h"
#include "ns3/ndnSIM/apps/bigdata-strategy.hpp"
#include "ns3/ndnSIM/apps/bigdata-default-strategy.hpp"
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"



namespace ns3 {

/**
 * This scenario simulates a very simple network topology:
 *
 *          (1)                        (2)                        (3)
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | StorageA | <------------> |StorageB| <------------> | StorageC |
 *      +----------+   1     10ms   +--------+    1     10ms  +----------+
 *            ^                         ^                          ^
 *            |1Mbps                    |  1Mbps                   |
 *            |                    10ms |  2                       | 1Mbps
 *        10ms|                         v                      10ms|
 *            |        1            +--------+        3            |
 *            +-------------------->| Admin  |<--------------------+
 *                                  +--------+
 *                                     (0)
 *
 *   (S-A) ----- ( ) ---- (S-B)
 *     |          |         |
 *    ( ) ------ ( ) ----- ( )
 *     |          |         |
 *    ( ) ----- (S-C) -- (Admin)
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 * ./waf --run="ndn-bigdata --replication=9 --nbStorages=9 --dimension=5 --segments=3 "
 *
 *     NS_LOG=ndn.Admin:ndn.Storage:ndn.DataConsumer ./waf --run=ndn-bigdata --command-template="gdb --args %s ./build/scratch/ndn-bigdata"
 */
int nextPrime(int a)
{
	int i, j, count;
	for (i = a + 1; 1; i++)
	{
		count = 0;
		for (j = 2;j < i; j++)
		{
			if (i%j == 0) // found a divisor
			{
				count++;
				break;  // break for (j = 2,j < i; j++) loop
				// this will speed up a bit
			}
		}
		if (count == 0)
		{
			return i;
			//break; // break for (i = a + 1; 1; i++) loop
			// break is not needed because return will stop execution of this function
		}
	}
}

int
main(int argc, char* argv[])
{
	uint32_t replication = 2;
	uint32_t nbStorages = 10;
	uint32_t dimension = 5;
	uint32_t nbSegments = 200;
	// std::to_string(nbStorages)
	// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	CommandLine cmd;
	cmd.AddValue ("replication", "Replication factor", replication);
	cmd.AddValue ("nbStorages", "Number of Storages", nbStorages);
	cmd.AddValue ("dimension", "Grid dimension", dimension);
	cmd.AddValue ("segments", "Number of segments per data file", nbSegments);
	cmd.Parse(argc, argv);

   //set the dumpfile location;
    //GlobalValue location("fileName","toto",replication);
    	static GlobalValue g_myGlobal =
  GlobalValue ("myGlobal", "Value for the dump file",
               StringValue (std::string("../../bigdata/dump-trace-")
		+std::to_string(replication)+std::string("_")
		+std::to_string(nbStorages)+std::string("_")
		+std::to_string(dimension)+std::string("_")
		+std::to_string(nbSegments)+std::string(".txt")),
               MakeStringChecker ());


	//AnnotatedTopologyReader topologyReader("", 25);
	// topologyReader.SetFileName("src/ndnSIM/apps/bigdata-topo-grid-3x3.txt");
	//topologyReader.Read();
	//
	//#    (0) ------ (S1) ----- ( 2)------ ( 3) ----- (S4)
	//#     |           |          |          |          |
	//#    ( 5) ------ (S6) ----- ( 7)------ ( 8) ----- ( 9)
	//#     |           |          |          |          |
	//#    (10) ------ (11) ----- (12)------ (13) ----- (S14)
	//#     |           |          |          |          |
	//#    (15) ------ (16) ----- (17)------ (18) ----- (S19)
	//#     |           |          |          |          |
	//#    (20) ------ (21) ----- (22)------ (23) ----- (24)

	const int max = (const int) dimension;

	// Creating 3x3 topology
	PointToPointHelper p2p;
	PointToPointGridHelper grid(max, max, p2p);
	grid.BoundingBox(100, 100, 200, 200);

	NodeContainer nodes;
	for (int i = 0; i < max; ++i) {
		for (int j = 0; j < max; ++j) {
			nodes.Add(grid.GetNode(i, j));
		}
	}



	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize",
			"100"); // default ContentStore parameters
	//	ndnHelper.SetOldContentStore("ns3::ndn::cs::Nocache");

	ndnHelper.InstallAll();

	// Set BestRoute strategy

	//topologyReader.ApplyOspfMetric();

	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	ndnGlobalRoutingHelper.InstallAll();

	ndn::StrategyChoiceHelper::InstallAll("/lacl/storage", "/localhost/nfd/strategy/bigdata");
	//ndn::StrategyChoiceHelper::InstallAll("/lacl/data", "/localhost/nfd/strategy/bigdata");
	//ndn::StrategyChoiceHelper::InstallAll("/lacl/data/heartbeat", "/localhost/nfd/strategy/multicast");
    ndn::StrategyChoiceHelper::InstallAll("/lacl/data", "/localhost/nfd/strategy/bigdatadefault");

	// Getting containers for the consumer/producer
	//Ptr<Node> producer = Names::Find<Node>("Node8");
	//NodeContainer nodes;
	//consumerNodes.Add(Names::Find<Node>("Node0"));

	ndn::AppHelper adminHelper("ns3::ndn::Admin");
	// Consumer will request /prefix/0, /prefix/1, ...
	adminHelper.SetAttribute("PrefixCommand",StringValue("/lacl/storage"));// /3/lacl/data/0/9
	adminHelper.SetAttribute("PrefixData",StringValue("/lacl/data"));
	adminHelper.SetAttribute("ReplicationFactor",StringValue(std::to_string(replication)));
	adminHelper.SetAttribute("LastSegment",StringValue(std::to_string(nbSegments)));
	adminHelper.Install(grid.GetNode(0, 0));//Names::Find<Node>("Node8"));                        // first node


	int lastPrime = 2;
	uint32_t countStorage = 0; //get the number of storage initialized
	// Storage A
	for (int i = 1; i < (int)dimension * (int)dimension; ++i) {
		if(i == lastPrime){
			lastPrime = nextPrime(lastPrime);
			continue;
		}
		ndn::AppHelper storageHelperA("ns3::ndn::Storage");
		storageHelperA.SetAttribute("PrefixCommand", StringValue("/lacl/storage"));
		storageHelperA.Install(nodes.Get(i));

        countStorage++;

		if(countStorage == nbStorages){
            break;      //break when we have the exact number of needed storages
		}

	}

	//	storageHelperA.Install(grid.GetNode(0, 0));//Names::Find<Node>("Node0")); // last node
	//	storageHelperA.Install(grid.GetNode(0, 2));//Names::Find<Node>("Node0")); // last node
	//	storageHelperA.Install(grid.GetNode(2, 1));//Names::Find<Node>("Node0")); // last node

	//	// Storage B
	//	ndn::AppHelper storageHelperB("ns3::ndn::Storage");
	//	storageHelperB.SetAttribute("PrefixCommand", StringValue("/lacl/storage"));
	//	storageHelperB.Install(Names::Find<Node>("Node2")); // last node
	//
	//	// Storage C
	//	ndn::AppHelper storageHelperC("ns3::ndn::Storage");
	//	storageHelperC.SetAttribute("PrefixCommand", StringValue("/lacl/storage"));
	//	storageHelperC.Install(Names::Find<Node>("Node7")); // last node

 /*  Simulator::Schedule (Seconds (15), ndn::LinkControlHelper::FailLink, nodes.Get (6), nodes.Get (1));
    Simulator::Schedule (Seconds (15), ndn::LinkControlHelper::FailLink, nodes.Get (6), nodes.Get (5));
    Simulator::Schedule (Seconds (15), ndn::LinkControlHelper::FailLink, nodes.Get (6), nodes.Get (7));
    Simulator::Schedule (Seconds (15), ndn::LinkControlHelper::FailLink, nodes.Get (6), nodes.Get (11));
*/
	Simulator::Stop(Seconds(1000.0));
	//ns3::Config::SetGlobal 	("toto", "../../bigdata/rate-trace-");



	ndn::L3RateTracer::InstallAll(std::string("../../bigdata/rate-trace-")
		+std::to_string(replication)+std::string("_")
		+std::to_string(nbStorages)+std::string("_")
		+std::to_string(dimension)+std::string("_")
		+std::to_string(nbSegments)+std::string(".txt"), Seconds(0.5));
	ndn::CsTracer::InstallAll(std::string("../../bigdata/cs-trace-")
		+std::to_string(replication)+std::string("_")
		+std::to_string(nbStorages)+std::string("_")
		+std::to_string(dimension)+std::string("_")
		+std::to_string(nbSegments)+std::string(".txt"), Seconds(0.5));

		ndn::AppDelayTracer::InstallAll(std::string("../../bigdata/delay-trace-")
		+std::to_string(replication)+std::string("_")
		+std::to_string(nbStorages)+std::string("_")
		+std::to_string(dimension)+std::string("_")
		+std::to_string(nbSegments)+std::string(".txt"));
	//  L2RateTracer::InstallAll("drop-trace.txt", Seconds(0.5));


	Simulator::Run();
	Simulator::Destroy();

	return 0;
}


} // namespace ns3

int
main(int argc, char* argv[])
{
	return ns3::main(argc, argv);
}

/*
 *
 * // setting default parameters for PointToPoint links and channels
	Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
	Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
	Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

	// Creating nodes
	NodeContainer nodes;
	nodes.Create(4);



	// Connecting nodes using two links
	PointToPointHelper p2p;
	p2p.Install(nodes.Get(0), nodes.Get(1));
	p2p.Install(nodes.Get(0), nodes.Get(2));
	p2p.Install(nodes.Get(0), nodes.Get(3));
	p2p.Install(nodes.Get(1), nodes.Get(2));
	p2p.Install(nodes.Get(2), nodes.Get(3));

	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	ndnHelper.SetDefaultRoutes(false);
	ndnHelper.InstallAll();

	// Choosing forwarding strategy best-route
	ndn::StrategyChoiceHelper::InstallAll("/lacl", "/localhost/nfd/strategy/best-route");


	//ndn::FibHelper::AddRoute(nodes.Get(0), "/", nodes.Get(1), std::numeric_limits<int32_t>::max());

	ndn::FibHelper::AddRoute(nodes.Get(0), "/lacl/storage", nodes.Get(1), 1);
	ndn::FibHelper::AddRoute(nodes.Get(0), "/lacl/storage", nodes.Get(2), 2);
	ndn::FibHelper::AddRoute(nodes.Get(0), "/lacl/storage", nodes.Get(3), 3);
	ndn::FibHelper::AddRoute(nodes.Get(1), "/lacl/storage", nodes.Get(2), 1);
	ndn::FibHelper::AddRoute(nodes.Get(2), "/lacl/storage", nodes.Get(3), 1);
	ndn::FibHelper::AddRoute(nodes.Get(3), "/lacl/storage", nodes.Get(2), 1);
	ndn::FibHelper::AddRoute(nodes.Get(2), "/lacl/storage", nodes.Get(1), 1);
	for (int n = 1; n < 4; ++n) {
		ndn::FibHelper::AddRoute(nodes.Get(0), "/lacl/data", nodes.Get(n), n);
		ndn::FibHelper::AddRoute(nodes.Get(0), "/lacl/code", nodes.Get(n), n);
		ndn::FibHelper::AddRoute(nodes.Get(n), "/lacl/data", nodes.Get(0), n);
		ndn::FibHelper::AddRoute(nodes.Get(n), "/lacl/code", nodes.Get(0), n);
	}

	ndn::FibHelper::AddRoute(nodes.Get(1), "/lacl/data", nodes.Get(2), 1);
	ndn::FibHelper::AddRoute(nodes.Get(2), "/lacl/data", nodes.Get(3), 1);
	ndn::FibHelper::AddRoute(nodes.Get(3), "/lacl/data", nodes.Get(2), 1);
	ndn::FibHelper::AddRoute(nodes.Get(2), "/lacl/data", nodes.Get(1), 1);

	ndn::FibHelper::AddRoute(nodes.Get(1), "/lacl/code", nodes.Get(2), 1);
	ndn::FibHelper::AddRoute(nodes.Get(2), "/lacl/code", nodes.Get(3), 1);
	ndn::FibHelper::AddRoute(nodes.Get(3), "/lacl/code", nodes.Get(2), 1);
	ndn::FibHelper::AddRoute(nodes.Get(2), "/lacl/code", nodes.Get(1), 1);
 *
 */


//ndn::StrategyChoiceHelper::InstallAll("/lacl/storage", "/localhost/nfd/strategy/best-route");
//ndn::StrategyChoiceHelper::InstallAll("/lacl/data", "/localhost/nfd/strategy/best-route");
//ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

// Installing applications

// Consumer
//    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
//    // Consumer will request /prefix/0, /prefix/1, ...
//    consumerHelper.SetPrefix("/lacl/data");
//    consumerHelper.SetAttribute("Frequency", StringValue("10")); // 10 interests a second
//    consumerHelper.Install(nodes.Get(3));                        // first node
//
//    // Producer
//    ndn::AppHelper producerHelper("ns3::ndn::Producer");
//    // Producer will reply to all requests starting with /prefix
//    producerHelper.SetPrefix("/prefix");
//    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
//    producerHelper.Install(nodes.Get(2)); // last node
//




// Consumer2
