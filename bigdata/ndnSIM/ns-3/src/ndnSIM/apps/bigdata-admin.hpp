#ifndef NDN_BIGDATA_ADMIN_H
#define NDN_BIGDATA_ADMIN_H
namespace ns3 {
namespace ndn {
class Admin;
}
}
#include "ndn-app.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "bigdata-dataconsumer.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include <list>

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief Ndn application for sending out Interest packets at a "constant" rate (Poisson process)
 */
class Admin: public App {
public:
	static TypeId
	GetTypeId();

	/**
	 * \brief Default constructor
	 * Sets up randomizer function and packet sequence number
	 */
	Admin();
	virtual ~Admin();
	// inherited from NdnApp
	virtual void
	OnInterest(shared_ptr<const Interest> interest);
    void OnInterestResponse(shared_ptr<const Interest> interest);

	// From App
	virtual void
	OnData(shared_ptr<const Data> contentObject);
	virtual void OnDataRetrieved(DataConsumer* consumer);
	virtual void OnTimeout(DataConsumer* consumer);
	int getLastSegment() const {
		return m_last_segment;
	}

	int getRepFactor() const {
		return m_rep_factor;
	}
	void setTransmittedInterests(shared_ptr<const Interest> interest) {
		m_transmittedInterests(interest,this,m_face);
	}
	TracedCallback<shared_ptr<const Interest>, Ptr<App>, shared_ptr<Face>> getTransmittedInterests() const {
		return m_transmittedInterests;
	}
	void setTransmittedDatas(shared_ptr<const Data> data) {
		m_transmittedDatas(data,this,m_face);
	}
	TracedCallback<shared_ptr<const Data>, Ptr<App>, shared_ptr<Face>> getTransmittedDatas() const {
		return m_transmittedDatas;
	}
//	void setFace(shared_ptr<AppFace> face) {
	void setFace(shared_ptr<Face> face) {
		m_face = face;
	}
	//shared_ptr<AppFace> getFace() const {
	shared_ptr<Face> getFace() const {
		return m_face;
	}
	void SendInterest(std::string prefix) ;
	Ptr<Node> getNode() const;
	Name getPrefixCommand() const {
			return m_prefix_command;
	}
	void pushConsumer(DataConsumer* dc) {
		consumers.push_back(dc);
	}
protected:
	// inherited from Application base class.
	virtual void
	StartApplication(); // Called at time specified by Start

	virtual void
	StopApplication(); // Called at time specified by Stop

protected:
	uint32_t m_rep_factor;
	uint32_t m_last_segment;
	Name m_prefix_command;
	Name m_prefix_data;
protected:
	uint32_t m_virtualPayloadSize = 1024;
	Time m_freshness;
	uint32_t m_signature = 100;
	Name m_keyLocator= Name("/unique/key/locator");
	std::list<DataConsumer*> consumers;

};

} // namespace ndn
} // namespace ns3

#endif
