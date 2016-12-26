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
	//this->replication = rest.substr(found+1, found2 - 1);

	this->replicationFactor = rest.substr(found+1, found2 - 1);

	string tmp = rest.substr(replicationFactor.size());

    size_t to = tmp.find_first_of("/");
    size_t to2 = tmp.find_first_of("/" , to + 1);

    this->replication = tmp.substr(to + 1, to2 -2);

	//this->replication = rest.substr(found2 + 1, found2 - 1);

	size_t found3 = rest.find_last_of("/");
	rest = rest.substr(0, found3);
	found3 = rest.find_last_of("/");
	this->lastSegment = rest.substr(found3+1);

	//rest = rest.substr(found2, found3 - 1);
    //rest = rest.substr(found2+2, found3 - 1);
    rest = 	rest.substr(replicationFactor.size() + replication.size()  +2 );

			// /lacl/data/0
	size_t found4 = rest.find_last_of("/");

	size_t found5 = rest.find_first_of("/");
//	this->dataPrefix = rest.substr(0, found4 - 1);
//    this->dataPrefix = rest.substr(0, found4 - 2);

     this->dataPrefix = rest.substr(found5, found4 -2);

}

StorageInfo::~StorageInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace ndn */
} /* namespace ns3 */
