/*
 * bigdata-storageinfo.hpp
 *
 *  Created on: Aug 19, 2016
 *      Author: ndn
 */

#ifndef BIGDATA_STORAGEINFO_HPP_
#define BIGDATA_STORAGEINFO_HPP_
namespace ns3 {
namespace ndn {
class StorageInfo;
}
}

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/simulator.h"
#include <sstream>
#include <string>
using namespace std;

namespace ns3 {
namespace ndn {

class StorageInfo {
protected:
	string replication;
	string lastSegment;
	string dataPrefix;
	string replicationFactor;
public:
	StorageInfo(string, string);
	virtual ~StorageInfo();

	void decrReplication() {
		setReplication(getReplicationAsInt() - 1);
	}
	string getDataPrefix() const {
		return dataPrefix;
	}

	void setDataPrefix(string dataPrefix) {
		this->dataPrefix = dataPrefix;
	}

	string getLastSegment() const {
		return lastSegment;
	}

	void setLastSegment(string lastSegment) {
		this->lastSegment = lastSegment;
	}

	int getLastSegmentAsInt() const {
		istringstream repReader(lastSegment);
		int lastSegmentVal = 0;
		repReader >> lastSegmentVal;
		return lastSegmentVal;
	}

	void setLastSegment(int lastSegment) {
		this->lastSegment = to_string(lastSegment);
	}

	string getReplication() const {
		return replication;
	}
	string getReplicationFactor() const {
		return replicationFactor;
	}

	int getReplicationAsInt() const {
		istringstream repReader(replication);
		int replicationValue = 0;
		repReader >> replicationValue;
		return replicationValue;
	}

	void setReplication(int replication) {
		this->replication = to_string(replication);
	}
	void setReplication(string replication) {
		this->replication = replication;
	}

	void setReplicationFactor(int replicationFactor) {
		this->replicationFactor = to_string(replicationFactor);
	}
	void setReplicationFactor(string replicationFactor) {
		this->replicationFactor = replicationFactor;
	}
};

} /* namespace ndn */
} /* namespace ns3 */
#endif /* BIGDATA_STORAGEINFO_HPP_ */
