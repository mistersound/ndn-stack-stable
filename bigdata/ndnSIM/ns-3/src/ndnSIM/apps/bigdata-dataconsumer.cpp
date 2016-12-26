/*
 * bigdata-dataconsumer.cpp
 *
 *  Created on: Aug 10, 2016
 *      Author: ndn
 */

#include "bigdata-dataconsumer.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"


#include "ns3/object.h"

#include "utils/ndn-ns3-packet-tag.hpp"
//#include "model/ndn-app-face.hpp"
#include "model/ndn-app-link-service.hpp"
#include "model/null-transport.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.DataConsumer");

namespace ns3 {
namespace ndn {

DataConsumer::DataConsumer(AppWrapper* a, Name interestName)

{
	wrapper = a;
	m_retxTimer = Time(50);
	m_rand = CreateObject<UniformRandomVariable>();

	m_seqMax = 10; // don't request anything
	m_interestName =interestName;
	m_rtt = CreateObject<RttMeanDeviation>();
	SetRetxTimer(Time(50));

}

DataConsumer::~DataConsumer() {
	Simulator::Cancel(m_sendEvent);
	Simulator::Cancel(m_retxEvent);

}


void
DataConsumer::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &DataConsumer::CheckRetxTimeout, this);
}

Time
DataConsumer::GetRetxTimer() const
{
  return m_retxTimer;
}

void
DataConsumer::CheckRetxTimeout()
{
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // timeout expired?
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &DataConsumer::CheckRetxTimeout, this);
}


void
DataConsumer::SendPacket()
{
	if (!m_active)
	return;

	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Sending Interest packet  " << m_interestName.toUri() );

	uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

	while (m_retxSeqs.size()) {
		seq = *m_retxSeqs.begin();
		m_retxSeqs.erase(m_retxSeqs.begin());
		break;
	}

	if (seq == std::numeric_limits<uint32_t>::max()) {
		if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
			if (m_seq >= m_seqMax) {
				//NS_LOG_DEBUG("> Exiting with seq = " << m_seq);
				NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Fire end of data collection   " << m_interestName.toUri() );
				Simulator::Schedule(Seconds(0.0), &AppWrapper::OnDataRetrieved, wrapper, this);
				m_active = false;
				return; // we are totally done
			}
		}

		seq = m_seq++;

	}

	//
	shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
	nameWithSequence->appendSequenceNumber(seq);
	//

	// shared_ptr<Interest> interest = make_shared<Interest> ();
//	shared_ptr<Interest> interest = make_shared<Interest>();
//	interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
//	interest->setName(*nameWithSequence);
//	time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
//	interest->setInterestLifetime(interestLifeTime);

	// NS_LOG_DEBUG ("Requesting Interest: \n" << *interest);
	NS_LOG_DEBUG("> Interest for " << seq);

	WillSendOutInterest(seq);
	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Requesting interest " << m_interestName.toUri() );

//	wrapper->setTransmittedInterests(interest);
//	wrapper->getFace()->onReceiveInterest(*interest);
	Simulator::Schedule(Seconds(0.0), &AppWrapper::SendInterest, wrapper, nameWithSequence->toUri());
	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Waiting for response " << m_interestName.toUri() );






	//ScheduleNextPacket();
}


void
DataConsumer::OnTimeout(uint32_t sequenceNumber)
{
	NS_LOG_FUNCTION(sequenceNumber);
	// std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
	// m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";
	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ")TimeOut fired " << m_interestName.toUri() );

	//check if the timeout is related to a heartbeat; or a storage

    if((m_interestName.toUri().find("heartbeat") != std::string::npos)||(m_interestName.toUri().find("storage") != std::string::npos)){

 //  NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") heartbeat Interest timeout  ");

    Simulator::Schedule(Seconds(0.0), &AppWrapper::OnTimeout, wrapper, this);

    }else{


	m_rtt->IncreaseMultiplier(); // Double the next RTO
	m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
			1); // make sure to disable RTT calculation for this sample
	m_retxSeqs.insert(sequenceNumber);
	//ScheduleNextPacket();Todo: add first packet call
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &DataConsumer::SendPacket, this);

    }
}

void
DataConsumer::WillSendOutInterest(uint32_t sequenceNumber)
{
	//NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
	//                              << m_seqTimeouts.size() << " items");

	m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
	m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

	m_seqLastDelay.erase(sequenceNumber);
	m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

	m_seqRetxCounts[sequenceNumber]++;

	m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}




int DataConsumer::OnData(shared_ptr<const Data> data) {

	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Checking received data packet" << data->getName().toUri()
			<< " against " << m_interestName.toUri());

	std::string prefix = m_interestName.toUri(); // /lacl/storage/2/lacl/data/0/3
	//std::size_t place = prefix.find_first_of(data->getName().toUri());	// /lacl/data/%FE%00

	std::size_t place = data->getName().toUri().find(prefix);
	if (place != 0) {
		NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Data packet is not relevant  " << data->getName().toUri() );
		return 0;
	}
	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") Data packet matched  " << data->getName().toUri() );

	// This could be a problem......
	uint32_t seq = data->getName().at(-1).toSequenceNumber();
	NS_LOG_DEBUG("< DATA for " << seq);

	int hopCount = 0;
	auto ns3PacketTag = data->getTag<Ns3PacketTag>();
	if (ns3PacketTag != nullptr) { // e.g., packet came from local node's cache
		FwHopCountTag hopCountTag;
		if (ns3PacketTag->getPacket()->PeekPacketTag(hopCountTag)) {
			hopCount = hopCountTag.Get();
			// NS_LOG_DEBUG("Hop count: " << hopCount);
		}
	}

	SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
	if (entry != m_seqLastDelay.end()) {
		m_lastRetransmittedInterestDataDelay(&(wrapper->getApp()), seq, Simulator::Now() - entry->time, hopCount);
	}

	entry = m_seqFullDelay.find(seq);
	if (entry != m_seqFullDelay.end()) {
		m_firstInterestDataDelay(&(wrapper->getApp()), seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
	}

	m_seqRetxCounts.erase(seq);
	m_seqFullDelay.erase(seq);
	m_seqLastDelay.erase(seq);

	m_seqTimeouts.erase(seq);
	m_retxSeqs.erase(seq);

	m_rtt->AckSeq(SequenceNumber32(seq));


	 m_sendEvent = Simulator::Schedule(Seconds(0.0), &DataConsumer::SendPacket, this);
	NS_LOG_DEBUG("node(" << wrapper->getNode()->GetId() << ") next packet scheduled  " << m_interestName.toUri() );

	return 1; /* TODO Test ici */
}




} /* namespace ndn */
} /* namespace ns3 */
