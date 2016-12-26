#include "bigdata-storage.hpp"
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

#include "helper/ndn-fib-helper.hpp"
//#include "model/ndn-app-face.hpp"
#include "model/ndn-app-link-service.hpp"

#include "model/ndn-ns3.hpp"
#include "ns3/node.h"
#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include <sstream>
#include <memory>

#include "ns3/log.h"
#include "ns3/simulator.h"

//#include "helper/ndn-global-routing-helper.hpp"
#include "ns3/ndnSIM-module.h"

NS_LOG_COMPONENT_DEFINE("ndn.Storage");



namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Storage);

TypeId
Storage::GetTypeId(void)
{
	static TypeId tid =
			TypeId("ns3::ndn::Storage")
			.SetGroupName("Ndn")
			.SetParent<App>()
			.AddConstructor<Storage>()

			.AddAttribute("PrefixCommand", "Prefix, for which producer has the data", StringValue("/"),
					MakeNameAccessor(&Storage::m_prefix_command), MakeNameChecker())
					// .AddAttribute("PrefixData", "Prefix, for which producer has the data", StringValue("/"),
					//              MakeNameAccessor(&Storage::m_prefix_data), MakeNameChecker())
					;

	return tid;
}

Storage::Storage()
{
}

Storage::~Storage()
{
}

Ptr<Node> Storage::getNode() const {
	return GetNode();
}
// inherited from Application base class.
void
Storage::StartApplication()
{
	NS_LOG_FUNCTION_NOARGS();
	App::StartApplication();

	//FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
	//this->state = new StorageFirstState(*this, GetNode(), m_prefix_command);
	NS_LOG_INFO("node(" << GetNode()->GetId() << ") Advertizing prefix " << m_prefix_command );
	FibHelper::AddRoute(GetNode(), m_prefix_command, m_face, 0);
	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	NodeContainer nodes;
	nodes.Add(GetNode());
	ndnGlobalRoutingHelper.AddOrigins(m_prefix_command.toUri(),nodes );
	ndn::GlobalRoutingHelper::CalculateRoutes();
	m_freshness = Seconds (10.0);
	NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage application started "  );
}

void
Storage::StopApplication()
{
	NS_LOG_FUNCTION_NOARGS();

	NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage application Stopping "  );

	for (std::list<DataConsumer*>::iterator it=consumers.begin(); it!=consumers.end(); ++it){
		delete (*it);
		consumers.erase(it);
	}
	App::StopApplication();
	NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage application stopped "  );
}

void Storage::OnDataRetrieved(DataConsumer* consumer){
	NS_LOG_INFO("node(" << GetNode()->GetId() << ") Data packet received " << consumer->getInterestName().toUri() );
	if (!m_active){
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage NOT active ignoring the packet " << consumer->getInterestName().toUri() );
		return;
	}

	if(!consumer->isCommand()){
		// consumer->getInterestName()  /lacl/data ?
		NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Data file " << consumer->getInterestName().toUri() <<" advertised and stored on the local storage" );
		FibHelper::AddRoute(GetNode(), consumer->getInterestName(), m_face , 0);
		ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
		NodeContainer nodes;
		nodes.Add(GetNode());
		ndnGlobalRoutingHelper.AddOrigins(consumer->getInterestName().toUri(), nodes);
		ndn::GlobalRoutingHelper::CalculateRoutes();
	}else {

	}

	if (consumer->isNeedsAction()) {
		std::string stop	= consumer->getInterestName().toUri()+"/stop/0/"+std::to_string(consumer->getSeqMax() - 1);
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") Creating a Stop command Consumer " << stop );
		DataConsumer* dataConsumer = new DataConsumer(new  AppWrapperTemplate<Storage*> (this), stop);
		dataConsumer->processCommand();
		//Simulator::Schedule(Seconds(0.0), &Storage::SendInterest, this, dataprefix);

	}


	NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Cleaning the Consumer "<< consumer->getInterestName().toUri() );
	consumers.remove(consumer);
	delete consumer;



}

void
Storage::OnData(shared_ptr<const Data> data)
{
	NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Data packet received " << data->getName().toUri() );
	if (!m_active){
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage NOT active ignoring the packet " << data->getName().toUri() );
		return;
	}
	App::OnData(data); // tracing inside



	bool consumed = false;
	for (std::list<DataConsumer*>::iterator it=consumers.begin(); it!=consumers.end(); ++it){
		if((*it)->OnData(data) == 1){
			NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Data consumed by  " << (*it)->getInterestName().toUri() );
			consumed = true;
			break;
		}
	}
	if(!consumed){
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") No consumer for data packet  " << data->getName().toUri() );
	}else{
		//		if(data->getName().toUri().find(m_prefix_command.toUri()) == 0) {
		//				NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Switching to Local Face 0 " << m_prefix_command.toUri());
		//				FibHelper::RemoveRoute(GetNode(), m_prefix_command, m_face );
		//				FibHelper::AddRoute(GetNode(), m_prefix_command, m_face, 0);
		//			}
	}


}
void Storage::SendInterest(std::string prefix) {
	NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Request for sending Interest " << prefix );
	if (!m_active){
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage NOT active ignoring the request " << prefix );
		return;
	}
	/////////////////////////////////////
	// Sending one Interest packet out //
	/////////////////////////////////////

	// Create and configure ndn::Interest
	auto interest = std::make_shared<ndn::Interest>(prefix);
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
	interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	interest->setInterestLifetime(ndn::time::seconds(10));

	NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Sending Interest packet for " << *interest);

	// Call trace (for logging purposes)
	m_transmittedInterests(interest, this, m_face);

//	m_face->onReceiveInterest(*interest);
	m_appLink->onReceiveInterest(*interest); 

}
void Storage::OnInterest(shared_ptr<const Interest> interest) {
	NS_LOG_INFO("node(" << GetNode()->GetId() << ") Interest packet received " << interest->getName().toUri() );
	if (!m_active){
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") Storage NOT active ignoring the packet " << interest->getName().toUri() );
		return;
	}

	App::OnInterest(interest); // tracing inside

	if (interest->getName().toUri().find(m_prefix_data.toUri()) != 0
			&& interest->getName().toUri().find(m_prefix_command.toUri()) != 0) {
		// case3: /autre/chose
		NS_LOG_INFO("node(" << GetNode()->GetId() << ") Ignoring unrecognized interest " << interest->getName().toUri()
				<< " against " << m_prefix_data.toUri());
		return ;
	}
	OnInterestResponse(interest);
	std::string prefix = m_prefix_command.toUri();
	std::size_t place = interest->getName().toUri().find(prefix);
	if (place == 0) {



		NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Interest is a storage command " << interest->getName().toUri() );
		StorageInfo storageInfo(interest->getName().toUri(), m_prefix_command.toUri());

		// decrement the replication value
		storageInfo.decrReplication();

		NS_LOG_DEBUG(
				"node(" << GetNode()->GetId()<< ") replication: "
				<< storageInfo.getReplication() << " lastSegment: "
				<< storageInfo.getLastSegment() << " dataprefix: "
				<< storageInfo.getDataPrefix());
		//TODO
		NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Creating a Data Consumer " << storageInfo.getDataPrefix() );
		DataConsumer* dataConsumer = new DataConsumer(new  AppWrapperTemplate<Storage*> (this), storageInfo.getDataPrefix());
		dataConsumer->processData(storageInfo);
		NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Data consumption event fired " << interest->getName().toUri() );

		m_prefix_data = Name(storageInfo.getDataPrefix());
		if (storageInfo.getReplicationAsInt() != 0) {
			// strategy forwards a new Interest to the lowest-cost nexthop (except downstream)

			//			NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Switching to Local Face to MAX_INT " << m_prefix_command);
			//			FibHelper::RemoveRoute(GetNode(), m_prefix_command, m_face );
			//			FibHelper::AddRoute(GetNode(), m_prefix_command, m_face, std::numeric_limits<uint32_t>::max());

			std::string replicationPrefix(m_prefix_command.toUri()
					+"/"+storageInfo.getReplication()+storageInfo.getDataPrefix()+"0/"+storageInfo.getLastSegment());
			NS_LOG_INFO("node(" << GetNode()->GetId() << ") Forwarding the replication command " << replicationPrefix );

			DataConsumer* repConsumer = new DataConsumer(new  AppWrapperTemplate<Storage*> (this), replicationPrefix);
			repConsumer->processCommand();
			NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Replication event fired " << replicationPrefix );
		}

		// cnp ce qui il y a en dessous
	} else {


		NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Interest is a Data retrieval " << interest->getName().toUri() );
	}
	//verifier si entre 0 et lastSegment




}
void Storage::OnInterestResponse(shared_ptr<const Interest> interest) {
	//verifier si entre 0 et lastSegment

	//si /stop/0/lastSegment remove advertise and return null
	Name dataName(interest->getName());
	// dataName.append(m_postfix);
	// dataName.appendVersion();

	auto data = make_shared<Data>();
	data->setName(dataName);
	data->setFreshnessPeriod(
			::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

	data->setContent(make_shared < ::ndn::Buffer > (m_virtualPayloadSize));
	NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") Preparing the data packet to serve "<< data->getName().toUri());

	Signature signature;
	SignatureInfo signatureInfo(
			static_cast<::ndn::tlv::SignatureTypeValue>(255));

	if (m_keyLocator.size() > 0) {
		signatureInfo.setKeyLocator(m_keyLocator);
	}

	signature.setInfo(signatureInfo);
	signature.setValue(
			::ndn::nonNegativeIntegerBlock(::ndn::tlv::SignatureValue,
					m_signature));

	data->setSignature(signature);

	//

	// to create real wire encoding
	data->wireEncode();

	setTransmittedDatas(data);
//	m_face->onReceiveData(*data);
	m_appLink->onReceiveData(*data);
	//state = &(state->OnInterest(interest));
	NS_LOG_DEBUG("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
}


} // namespace ndn
} // namespace ns3
