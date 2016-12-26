/*
 * bigdata-dataconsumer.h
 *
 *  Created on: Aug 10, 2016
 *      Author: ndn
 */

#ifndef BIGDATA_DATACONSUMER_H_
#define BIGDATA_DATACONSUMER_H_
namespace ns3 {
namespace ndn {

class DataConsumer;
}
}
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/random-variable-stream.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"

#include "ns3/ndnSIM/utils/ndn-rtt-estimator.hpp"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.hpp"

#include <set>
#include <map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "ndn-app.hpp"
#include <ns3/random-variable-stream.h>
#include "bigdata-storage.hpp"
#include "bigdata-storageinfo.hpp"
#include "bigdata-wrapper.hpp"

namespace ns3 {
namespace ndn {

class DataConsumer {
public:
	DataConsumer(AppWrapper* a, Name interestName);
	virtual ~DataConsumer();
	virtual int OnData(shared_ptr<const Data> contentObject);
	void SendPacket();
	void OnTimeout(uint32_t sequenceNumber);
	void WillSendOutInterest(uint32_t sequenceNumber);

	bool isCommand() const
	{
		return m_command;
	}

	void setCommand(bool command = false)
	{
		m_command = command;
	}

	Name getInterestName() const
	{
		return m_interestName;
	}

	uint32_t getSeqMax() const
	{
		return m_seqMax;
	}

	void setSeqMax(uint32_t seqMax)
	{
		m_seqMax = seqMax;
	}

	void setNeedsAction(bool status)
	{
		this->status = status;
	}

	bool isNeedsAction() const
	{
		return status;
	}

	void setStopInterestDone(bool value)
	{
		this->stopInterestDone = value;
	}

	bool isStopInterestDone() const
	{
		return stopInterestDone;
	}






	  /**
	   * \brief Checks if the packet need to be retransmitted becuase of retransmission timer expiration
	   */
	  void
	  CheckRetxTimeout();

	  /**
	   * \brief Modifies the frequency of checking the retransmission timeouts
	   * \param retxTimer Timeout defining how frequent retransmission timeouts should be checked
	   */
	  void
	  SetRetxTimer(Time retxTimer);

	  /**
	   * \brief Returns the frequency of checking the retransmission timeouts
	   * \return Timeout defining how frequent retransmission timeouts should be checked
	   */
	  Time
	  GetRetxTimer() const;

	  void processData(StorageInfo& storageInfo) {
	  			setSeqMax(storageInfo.getLastSegmentAsInt() + 1);
	  			setNeedsAction(storageInfo.getReplicationAsInt() == 0);
	  			if(isStopInterestDone()){
                    setNeedsAction(false);
	  			}
	  			wrapper->pushConsumer(this);
	  			Simulator::Schedule(Seconds(0.0), &DataConsumer::SendPacket, this);
	  }
	  void processCommand() {
	  			setSeqMax(1);
	  			setCommand(true);
	  			wrapper->pushConsumer(this);
	  			Simulator::Schedule(Seconds(0.0), &DataConsumer::SendPacket, this);
	  }

protected:
	bool m_active = true;
	AppWrapper* wrapper;
	Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator
	uint32_t m_seq = 0;///< @brief currently requested sequence number
	uint32_t m_seqMax;///< @brief maximum number of sequence number
	EventId m_sendEvent;///< @brief EventId of pending "send packet" event
	Time m_retxTimer;///< @brief Currently estimated retransmission timer
	EventId m_retxEvent;///< @brief Event to check whether or not retransmission should be performed
	Ptr<RttEstimator> m_rtt;///< @brief RTT estimator
	Time m_offTime;///< \brief Time interval between packets
	Name m_interestName;///< \brief NDN Name of the Interest (use Name)
	Time m_interestLifeTime = Time(1);///< \brief LifeTime for interest packet , m_interestLifeTime(Time(1))
	bool status = false;
	bool m_command = false;
	bool stopInterestDone = false; // Use to know when the last replication factor storage node send the stop Interest
	/// @cond include_hidden
	/**
	 * \struct This struct contains sequence numbers of packets to be retransmitted
	 */
	struct RetxSeqsContainer : public std::set<uint32_t>
	{
	};
	RetxSeqsContainer m_retxSeqs; ///< \brief ordered set of sequence numbers to be retransmitted
	/**
	 * \struct This struct contains a pair of packet sequence number and its timeout
	 */
	struct SeqTimeout
	{
		SeqTimeout(uint32_t _seq, Time _time)
		:seq(_seq), time(_time)
		{
		}

		uint32_t seq;
		Time time;
	};
	/// @endcond
	/// @cond include_hidden
	class i_seq
	{
	};
	class i_timestamp
	{
	};
	/// @endcond
	/// @cond include_hidden
	/**
	 * \struct This struct contains a multi-index for the set of SeqTimeout structs
	 */
	struct SeqTimeoutsContainer : public boost::multi_index::multi_index_container<SeqTimeout,boost::multi_index::indexed_by<boost::multi_index::ordered_unique<boost::multi_index::tag<i_seq> ,boost::multi_index::member<SeqTimeout,uint32_t,&SeqTimeout::seq> > ,boost::multi_index::ordered_non_unique<boost::multi_index::tag<i_timestamp> ,boost::multi_index::member<SeqTimeout,Time,&SeqTimeout::time> > > >
	{
	};
	SeqTimeoutsContainer m_seqTimeouts; ///< \brief multi-index for the set of SeqTimeout structs
	SeqTimeoutsContainer m_seqLastDelay;
	SeqTimeoutsContainer m_seqFullDelay;
	std::map<uint32_t,uint32_t> m_seqRetxCounts;
	TracedCallback<Ptr<App> /* app */
	,uint32_t/* seqno */
	,Time/* delay */
	,int32_t/*hop count*/
	> m_lastRetransmittedInterestDataDelay;
	TracedCallback<Ptr<App> /* app */
	,uint32_t/* seqno */
	,Time/* delay */
	,uint32_t/*retx count*/
	,int32_t/*hop count*/
	> m_firstInterestDataDelay;
	/// @endcond






};

} /* namespace ndn */
} /* namespace ns3 */
#endif /* BIGDATA_DATACONSUMER_H_ */
