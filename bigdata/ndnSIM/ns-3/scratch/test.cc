#include <iostream>

//core module and required helpers for example
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

//required for use of CS (from NFD)
#include "ns3/ndnSIM/NFD/daemon/table/cs.hpp"
#include "ns3/ndnSIM/NFD/daemon/table/cs-policy-priority-fifo.hpp"
#include "ns3/ndnSIM/NFD/core/logger.hpp"
#include "ns3/ndnSIM/NFD/core/algorithm.hpp"
#include "ns3/ndnSIM/ndn-cxx/data.hpp"
//#include "umd/JPBPolicyToFile.hpp"

//#include <memory>   //for the unique_ptr and make_unique
#include <string>
#include <array>
using namespace std;

namespace ns3 {


  int
  main(int argc, char* argv[])
  {

     //setting default parameters for point to point links & channels

     Config::SetDefault("ns3::PointToPointNetDevice::DataRate",
                        StringValue("1Mbps"));
     Config::SetDefault("ns3::PointToPointChannel::Delay",
                        StringValue("10ms"));
     Config::SetDefault("ns3::DropTailQueue::MaxPackets",
                        StringValue("20"));

     //read optional command line params
     CommandLine cmd;
     cmd.Parse(argc, argv);

     //Create the node
     //through the node container

     NodeContainer nodes;
     nodes.Create(3);  // [consumer] -- [router] -- [producer]

     //connect nodes (create channels)
     PointToPointHelper p2p;
     p2p.Install(nodes.Get(0), nodes.Get(1));
     p2p.Install(nodes.Get(1), nodes.Get(2));

     //install the NDN stack on the node
     ndn::StackHelper ndnHelper;
     ndnHelper.SetDefaultRoutes(true);
     ndnHelper.InstallAll();

     //install forwarding strategy on the node
     ndn::StrategyChoiceHelper::InstallAll("/prefix",
                                   "/localhost/nfd/strategy/multicast");

     //for a node - the router (node 1)
     Ptr<ns3::ndn::L3Protocol> l3 =
                        ns3::ndn::L3Protocol::getL3Protocol(nodes.Get(0));

     //get the content store for the node
     nfd::Cs& myContentStore = l3->getForwarder()->getCs();
     //myContentStore.setLimit(10);

     //-----------| set policy for Cs

    cout<<nodes.Get(1)<<endl<<endl;
     //unique_ptr<JPBPolicyToFile> policy =  ndn::make_unique<JPBPolicyToFile>();


     //myContentStore.setPolicy(move(policy));

     //consumer
     ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
     consumerHelper.SetPrefix("/prefix");
     consumerHelper.SetAttribute("Frequency",
                                 StringValue("10"));
     consumerHelper.Install(nodes.Get(0));

     //producer
     ndn::AppHelper producerHelper("ns3::ndn::Producer");
     producerHelper.SetPrefix("/prefix");
     producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
     producerHelper.Install(nodes.Get(2));

     //run simulation
     Simulator::Stop(Seconds(20.0));
     Simulator::Run();

     //print from content store
     //to make sure the items live there
     cout<<"ABOUT TO PRINT the Content Store data items"<<endl;
     for(auto it = myContentStore.begin();
         it != myContentStore.end();
         ++it)
     {
        cout<<it->getName()<<endl<<endl;
     }


     Simulator::Destroy();
     return 0;
  }
} //end namespace

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
