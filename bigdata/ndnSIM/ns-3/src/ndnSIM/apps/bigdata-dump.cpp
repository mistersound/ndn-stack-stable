/*
 * bigdata-dumpinfo.cpp
 *
 *  Created on: Dec 21, 2016
 *      Author: ndn
 */

#include "bigdata-dump.hpp"

namespace ns3 {
namespace ndn {

DumpInfo::DumpInfo(string file) {

	this->setLocation(file);

}

DumpInfo::DumpInfo(){
}

DumpInfo::~DumpInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace ndn */
} /* namespace ns3 */
