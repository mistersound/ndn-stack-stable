

#include "bigdata-strategy.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <ndn-cxx/util/random.hpp>

#include "core/logger.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <ns3/simulator.h>
#include "ns3/core-module.h"

NFD_LOG_INIT("BigDataStrategy");


using namespace std;

namespace nfd {
namespace fw {
const Name
BigDataStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/bigdata");
NFD_REGISTER_STRATEGY(BigDataStrategy);

BigDataStrategy::BigDataStrategy(Forwarder& forwarder, const Name& name)
: Strategy(forwarder, name)
{

}

BigDataStrategy::~BigDataStrategy()
{

}


/** \brief determines whether a NextHop is eligible
 *  \param currentDownstream incoming FaceId of current Interest
 *  \param wantUnused if true, NextHop must not have unexpired OutRecord
 *  \param now time::steady_clock::now(), ignored if !wantUnused
 */
static inline bool
predicate_NextHop_eligible(bool rep,const shared_ptr<pit::Entry>& pitEntry,
		const fib::NextHop& nexthop, FaceId currentDownstream,
		bool wantUnused = false,
		time::steady_clock::TimePoint now = time::steady_clock::TimePoint::min())
{
	shared_ptr<Face> upstream = nexthop.getFace();



	NFD_LOG_DEBUG("Testing Face" + upstream->getRemoteUri().toString());
	//if(rep && upstream->isLocal()){
	if(rep && (upstream->getScope() == ndn::nfd::FACE_SCOPE_LOCAL)){
		NFD_LOG_DEBUG("Face rejected " + upstream->getRemoteUri().toString());
		return false;
	}

	// upstream is current downstream
	if (upstream->getId() == currentDownstream)
		return false;

	// forwarding would violate scope
	if (pitEntry->violatesScope(*upstream))
		return false;

	if (wantUnused) {
		// NextHop must not have unexpired OutRecord
		pit::OutRecordCollection::const_iterator outRecord = pitEntry->getOutRecord(*upstream);
		if (outRecord != pitEntry->getOutRecords().end() &&
				outRecord->getExpiry() > now) {
			return false;
		}
	}

	return true;
}

/** \brief pick an eligible NextHop with earliest OutRecord
 *  \note It is assumed that every nexthop has an OutRecord
 */
static inline fib::NextHopList::const_iterator
findEligibleNextHopWithEarliestOutRecord(bool rep,const shared_ptr<pit::Entry>& pitEntry,
		const fib::NextHopList& nexthops,
		FaceId currentDownstream)
{
	fib::NextHopList::const_iterator found = nexthops.end();
	time::steady_clock::TimePoint earliestRenewed = time::steady_clock::TimePoint::max();
	for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {
		if (!predicate_NextHop_eligible(rep ,pitEntry, *it, currentDownstream))
			continue;
		pit::OutRecordCollection::const_iterator outRecord = pitEntry->getOutRecord(*it->getFace());
		BOOST_ASSERT(outRecord != pitEntry->getOutRecords().end());
		if (outRecord->getLastRenewed() < earliestRenewed) {
			found = it;
			earliestRenewed = outRecord->getLastRenewed();
		}
	}
	return found;
}

void
BigDataStrategy::afterReceiveInterest(const Face& inFace,
		const Interest& interest,
		shared_ptr<fib::Entry> fibEntry,
		shared_ptr<pit::Entry> pitEntry)
{



   ns3::StringValue stringValue;
   ns3::GlobalValue::GetValueByName ("myGlobal", stringValue);
   string file = stringValue.Get ();



    std::ofstream outfile;
   outfile.open(file, std::ios_base::app);
   NFD_LOG_DEBUG("INFORMATION::::::::" << interest.toUri() );

    outfile << ns3::Simulator::Now().GetSeconds() << ";" << ns3::Simulator::GetContext () << ";" << interest.toUri() << std::endl;


    outfile.close();



	const fib::NextHopList& nexthops = fibEntry->getNextHops();
	fib::NextHopList::const_iterator it = nexthops.end();

	RetxSuppression::Result suppression =
			m_retxSuppression.decide(inFace, interest, *pitEntry);
	if (suppression == RetxSuppression::NEW) {
		// forward to nexthop with lowest cost except downstream
		it = std::find_if(nexthops.begin(), nexthops.end(),
				bind(&predicate_NextHop_eligible,m_replicated, pitEntry, _1, inFace.getId(),
						false, time::steady_clock::TimePoint::min()));

		if (it == nexthops.end()) {
			NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");
			this->rejectPendingInterest(pitEntry);
			return;
		}

		// Check if this is a storage request
		if (! m_replicated) {
			std::string prefix = "/lacl/storage";
			std::size_t place = interest.getName().toUri().find(prefix);
			if (place == 0) {
				NFD_LOG_DEBUG("Replication registered for " << interest.getName().toUri() << " from=" << inFace.getId());
				NFD_LOG_INFO("Replication Request on " << inFace.getRemoteUri().toString() << " for " << interest.getName().toUri());
				m_replicated = true;

			}
		}

		shared_ptr<Face> outFace = it->getFace();
		this->sendInterest(pitEntry, outFace);
		NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
				<< " newPitEntry-to=" << outFace->getId());
		return;
	}

	if (suppression == RetxSuppression::SUPPRESS) {
		NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
				<< " suppressed");
		return;
	}

	// find an unused upstream with lowest cost except downstream
	it = std::find_if(nexthops.begin(), nexthops.end(),
			bind(&predicate_NextHop_eligible, m_replicated, pitEntry, _1, inFace.getId(),
					true, time::steady_clock::now()));
	if (it != nexthops.end()) {
		shared_ptr<Face> outFace = it->getFace();
		this->sendInterest(pitEntry, outFace);
		NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
				<< " retransmit-unused-to=" << outFace->getId());
		return;
	}

	// find an eligible upstream that is used earliest
	it = findEligibleNextHopWithEarliestOutRecord(m_replicated, pitEntry, nexthops, inFace.getId());
	if (it == nexthops.end()) {
		NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " retransmitNoNextHop");
	}
	else {
		shared_ptr<Face> outFace = it->getFace();
		this->sendInterest(pitEntry, outFace);
		NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
				<< " retransmit-retry-to=" << outFace->getId());
	}



}

} // namespace fw
} // namespace nfd





/*



static inline bool
predicate_PitEntry_canForwardTo_NextHop(bool rep, shared_ptr<pit::Entry> pitEntry,
		const fib::NextHop& nexthop)
{
	NFD_LOG_INFO("Testing Face" + nexthop.getFace()->getRemoteUri().toString());
	if(rep && nexthop.getFace()->isLocal()){
		NFD_LOG_INFO("Face rejected " + nexthop.getFace()->getRemoteUri().toString());
		return false;
	}

	return pitEntry->canForwardTo(*nexthop.getFace());
}

void
BigDataStrategy::afterReceiveInterest(const Face& inFace,
		const Interest& interest,
		shared_ptr<fib::Entry> fibEntry,
		shared_ptr<pit::Entry> pitEntry)
{
	if (pitEntry->hasUnexpiredOutRecords()) {
		// not a new Interest, don't forward
		return;
	}

	const fib::NextHopList& nexthops = fibEntry->getNextHops();
	fib::NextHopList::const_iterator it = std::find_if(nexthops.begin(), nexthops.end(),
			bind(&predicate_PitEntry_canForwardTo_NextHop,m_replicated, pitEntry, _1));

	if (it == nexthops.end()) {
		this->rejectPendingInterest(pitEntry);
		return;
	}

	shared_ptr<Face> outFace = it->getFace();

	std::string prefix = "/lacl/storage";
	std::size_t place = interest.getName().toUri().find(prefix);
	if (place == 0) {
		NFD_LOG_INFO("Replication registered for "+ interest.getName().toUri());


		if (! m_replicated) {
			NFD_LOG_INFO("Replication Request on " + inFace.getRemoteUri().toString() + " for " + interest.getName().toUri());
			m_replicated = true;
		}else{
			NFD_LOG_INFO("Replication request "+ interest.getName().toUri() + " forwarded to " + outFace->getRemoteUri().toString());
			outFace->setMetric(outFace->getMetric()+2);
		}

	}




	this->sendInterest(pitEntry, outFace);
}


 */
