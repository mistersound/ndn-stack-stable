/*
 * bigdata-storageinfo.cpp
 *
 *  Created on: Aug 19, 2016
 *      Author: ndn
 */

#include "bigdata-storageinfo.hpp"

namespace ns3 {
namespace ndn {

StorageInfo::StorageInfo(string interest, string prefix) {
	// case1: /lacl/storage/3/lacl/data/0/9/XX
	string rest = interest.substr(prefix.size());
	size_t found = rest.find_first_of("/");
	size_t found2 = rest.find_first_of("/", found + 1);
	this->replication = rest.substr(found+1, found2 - 1);
	size_t found3 = rest.find_last_of("/");
	rest = rest.substr(0, found3);
	found3 = rest.find_last_of("/");
	this->lastSegment = rest.substr(found3+1);

	rest = rest.substr(found2, found3 - 1);
			// /lacl/data/0
	size_t found4 = rest.find_last_of("/");
	this->dataPrefix = rest.substr(0, found4 - 1);

}

StorageInfo::~StorageInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace ndn */
} /* namespace ns3 */
