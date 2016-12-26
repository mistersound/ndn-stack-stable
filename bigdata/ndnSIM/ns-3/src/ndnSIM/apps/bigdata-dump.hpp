/*
 * bigdata-dumpInfo.hpp
 *
 *  Created on: dec 21, 2016
 *      Author: ndn
 */

#ifndef BIGDATA_DUMP_HPP_
#define BIGDATA_DUMP_HPP_
namespace ns3 {
namespace ndn {
class DumpInfo;
}
}


#include <sstream>
#include <string>
using namespace std;

namespace ns3 {
namespace ndn {

class DumpInfo {
protected:
	string  location;
public:
    DumpInfo();
	DumpInfo(string);
	virtual ~DumpInfo();



	void setLocation(string loc) {
		this->location = loc;
	}

	string getLocation() const {
		return location;
	}


};

} /* namespace ndn */
} /* namespace ns3 */
#endif /* BIGDATA_STORAGEINFO_HPP_ */
