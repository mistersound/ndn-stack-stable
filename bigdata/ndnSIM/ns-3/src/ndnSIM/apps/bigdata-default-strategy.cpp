

#include "bigdata-default-strategy.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <ndn-cxx/util/random.hpp>

#include "core/logger.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <ns3/simulator.h>
#include "ns3/core-module.h"

NFD_LOG_INIT("BigDataDefaultStrategy");

using namespace std;

namespace nfd {
namespace fw {

const Name
BigDataDefaultStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/bigdatadefault");
NFD_REGISTER_STRATEGY(BigDataDefaultStrategy);

BigDataDefaultStrategy::BigDataDefaultStrategy(Forwarder& forwarder, const Name& name)
: BestRouteStrategy(forwarder, name)
{

}

BigDataDefaultStrategy::~BigDataDefaultStrategy()
{

}



void
BigDataDefaultStrategy::afterReceiveInterest(const Face& inFace,
		const Interest& interest,
		shared_ptr<fib::Entry> fibEntry,
		shared_ptr<pit::Entry> pitEntry)
{

   ns3::StringValue stringValue;
   ns3::GlobalValue::GetValueByName ("myGlobal", stringValue);
   string file = stringValue.Get ();

    BestRouteStrategy::afterReceiveInterest(inFace,interest,fibEntry,pitEntry);

    std::ofstream outfile;
    outfile.open(file, std::ios_base::app);

    NFD_LOG_DEBUG("INFORMATION::::::::" << interest.toUri() );

    outfile << ns3::Simulator::Now().GetSeconds() << ";" << ns3::Simulator::GetContext () << ";" << interest.toUri() << std::endl;


    outfile.close();

		return;
	}



} // namespace fw
} // namespace nfd

