#include "bigdata-random-strategy.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <ndn-cxx/util/random.hpp>

#include "core/logger.hpp"

NFD_LOG_INIT("BigdataRandomStrategy");

namespace nfd {
namespace fw {

const Name
BigdataRandomStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/random-bigdata");
NFD_REGISTER_STRATEGY(BigdataRandomStrategy);

BigdataRandomStrategy::BigdataRandomStrategy(Forwarder& forwarder, const Name& name)
: Strategy(forwarder, name)
{
}

BigdataRandomStrategy::~BigdataRandomStrategy()
{
}

static bool
canForwardToNextHop(bool rep, shared_ptr<pit::Entry> pitEntry, const fib::NextHop& nexthop)
{
	//NFD_LOG_INFO("Testing Face" + nexthop.getFace()->getRemoteUri().toString());
		//if(rep && nexthop.getFace()->isLocal()){
		if(rep && (nexthop.getFace()->getScope() == ndn::nfd::FACE_SCOPE_LOCAL)){
		//	NFD_LOG_INFO("Face rejected " + nexthop.getFace()->getRemoteUri().toString());
			return false;
		}
	return pitEntry->canForwardTo(*nexthop.getFace());
}

static bool
hasFaceForForwarding(bool rep, const fib::NextHopList& nexthops, shared_ptr<pit::Entry>& pitEntry)
{
	return std::find_if(nexthops.begin(), nexthops.end(), bind(&canForwardToNextHop, rep, pitEntry, _1))
	!= nexthops.end();
}

void
BigdataRandomStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
		shared_ptr<fib::Entry> fibEntry,
		shared_ptr<pit::Entry> pitEntry)
{
	//NFD_LOG_TRACE("afterReceiveInterest");
	NFD_LOG_TRACE("Forwarding request for interest " << interest.getName().toUri() << " from=" << inFace.getId());


	if (pitEntry->hasUnexpiredOutRecords()) {
		// not a new Interest, don't forward
		return;
	}

	const fib::NextHopList& nexthops = fibEntry->getNextHops();

	// Ensure there is at least 1 Face is available for forwarding
	if (!hasFaceForForwarding(m_replicated, nexthops, pitEntry)) {
		this->rejectPendingInterest(pitEntry);
		return;
	}

	fib::NextHopList::const_iterator selected;
	do {
		boost::random::uniform_int_distribution<> dist(0, nexthops.size() - 1);
		const size_t randomIndex = dist(m_randomGenerator);

		uint64_t currentIndex = 0;

		for (selected = nexthops.begin(); selected != nexthops.end() && currentIndex != randomIndex;
				++selected, ++currentIndex) {
		}
	} while (!canForwardToNextHop(m_replicated, pitEntry, *selected));

	// Check if this is a storage request
	//if (! m_replicated && selected->getFace()->isLocal()) {
	if (! m_replicated && (selected->getFace()->getScope() == ndn::nfd::FACE_SCOPE_LOCAL)) {
		std::string prefix = "/lacl/storage";
		std::size_t place = interest.getName().toUri().find(prefix);
		if (place == 0) {
			NFD_LOG_INFO("Replication registered for " << interest.getName().toUri() << " from=" << inFace.getId());
			//NFD_LOG_INFO("Replication Request on " << inFace.getRemoteUri().toString() << " for " << interest.getName().toUri());
			m_replicated = true;

		}
	}
	NFD_LOG_TRACE("Forwarding request for interest " << interest.getName().toUri() << " to=" << selected->getFace()->getId());
	this->sendInterest(pitEntry, selected->getFace());
}

} // namespace fw
} // namespace nfd
