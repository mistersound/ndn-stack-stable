#ifndef NDN_BIGDATA_STORAGE_H
#define NDN_BIGDATA_STORAGE_H
namespace ns3 {
namespace ndn {
class Storage;
}
}
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-app-link-service.hpp"
#include "bigdata-dataconsumer.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "bigdata-storageinfo.hpp"
#include <list>

namespace ns3 {
namespace ndn {

class Storage : public App {
public:
	static TypeId
	GetTypeId();

	/**
	 * \brief Default constructor
	 * Sets up randomizer function and packet sequence number
	 */
	Storage();
	virtual ~Storage();
	void SendInterest(std::string prefix) ;
	// (overridden from ndn::App) Callback that will be called when Interest arrives
	  virtual void
	  OnInterest(std::shared_ptr<const ndn::Interest> interest);
	  void OnInterestResponse(shared_ptr<const Interest> interest);

	  // (overridden from ndn::App) Callback that will be called when Data arrives
	  virtual void
	  OnData(std::shared_ptr<const ndn::Data> contentObject);
	virtual void OnDataRetrieved(DataConsumer* consumer);
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
	shared_ptr<Face> getFace() const {
		return m_face;
	}
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

	Name m_prefix_command;
	Name m_prefix_data= Name("NO_DATA");

protected:


	uint32_t m_virtualPayloadSize = 1024;
	Time m_freshness;
	uint32_t m_signature = 100;
	Name m_keyLocator = Name("/unique/key/locator");
	//std::map<std::string, DataConsumer*> consumers;
	std::list<DataConsumer*> consumers;
};


} // namespace ndn
} // namespace ns3

#endif // NDN_Storage_H
