/*
 * bigdata-wrapper.hpp
 *
 *  Created on: Aug 12, 2016
 *      Author: ndn
 */

#ifndef BIGDATA_WRAPPER_HPP_
#define BIGDATA_WRAPPER_HPP_
namespace ns3 {
namespace ndn {
class AppWrapper;
}
}
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "bigdata-dataconsumer.h"
#include "ndn-app.hpp"

namespace ns3 {
namespace ndn {
class AppWrapper {
public:
	AppWrapper() {}
	virtual ~AppWrapper() {}
	virtual void OnInterest(shared_ptr<const Interest> interest) =0;
	virtual void OnData(shared_ptr<const Data> contentObject) =0;
	virtual void setTransmittedInterests(shared_ptr<const Interest> interest) =0;
	virtual TracedCallback<shared_ptr<const Interest>, Ptr<App>, shared_ptr<Face>> getTransmittedInterests() const =0;
	virtual void setTransmittedDatas(shared_ptr<const Data> data) =0;
	virtual TracedCallback<shared_ptr<const Data>, Ptr<App>, shared_ptr<Face>> getTransmittedDatas() const =0;
	//virtual void setFace(shared_ptr<AppFace> face) =0;
	virtual shared_ptr<Face> getFace() const =0;
	virtual App& getApp() const =0;
	virtual Ptr<Node> getNode() const = 0;
	virtual void OnDataRetrieved(DataConsumer* consumer) =0;
	virtual void SendInterest(std::string prefix) =0;
	virtual Name getPrefixCommand() const = 0;
	virtual void pushConsumer(DataConsumer* dc) = 0;
};
template<class Item>
class AppWrapperTemplate : public AppWrapper {
protected:
	Item item;
public:
	AppWrapperTemplate(Item i) : item(i) {

	}
	virtual ~AppWrapperTemplate() {

	}
	virtual void OnInterest(shared_ptr<const Interest> interest) {
		item->OnInterest(interest);
	}
	// From App
	virtual void OnData(shared_ptr<const Data> contentObject) {
		item->OnData(contentObject);
	}
	void setTransmittedInterests(shared_ptr<const Interest> interest) {
		item->setTransmittedInterests(interest);
	}
	TracedCallback<shared_ptr<const Interest>, Ptr<App>, shared_ptr<Face>> getTransmittedInterests() const {
		return item->getTransmittedInterests();
	}
	void setTransmittedDatas(shared_ptr<const Data> data) {
		item->setTransmittedDatas(data);
	}
	TracedCallback<shared_ptr<const Data>, Ptr<App>, shared_ptr<Face>> getTransmittedDatas() const {
		return item->getTransmittedDatas();
	}
	void setFace(shared_ptr<Face> face) {
		item->setFace(face);;
	}
	shared_ptr<Face> getFace() const {
		return item->getFace();
	}
	Ptr<Node> getNode() const {
		return item->getNode();
	}
	App& getApp() const {
		return *item;
	}
	void OnDataRetrieved(DataConsumer* consumer) {
		return item->OnDataRetrieved(consumer);
	}
	void SendInterest(std::string prefix) {
		item->SendInterest(prefix) ;
	}
	Name getPrefixCommand() const {
			return item->getPrefixCommand();
	}
	void pushConsumer(DataConsumer* dc) {
		item->pushConsumer(dc);
	}
};

} /* namespace ndn */
} /* namespace ns3 */
#endif /* BIGDATA_WRAPPER_HPP_ */
