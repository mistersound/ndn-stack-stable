#ifndef BIGDATA_RANDOM_STRATEGY_HPP
#define BIGDATA_RANDOM_STRATEGY_HPP

#include <boost/random/mersenne_twister.hpp>
#include "face/face.hpp"
#include "fw/strategy.hpp"

namespace nfd {
namespace fw {

class BigdataRandomStrategy : public Strategy {
public:
	BigdataRandomStrategy(Forwarder& forwarder, const Name& name = STRATEGY_NAME);

  virtual ~BigdataRandomStrategy();

  virtual void
  afterReceiveInterest(const Face& inFace, const Interest& interest,
                       shared_ptr<fib::Entry> fibEntry, shared_ptr<pit::Entry> pitEntry);

public:
  static const Name STRATEGY_NAME;

protected:
  bool m_replicated = false;
  boost::random::mt19937 m_randomGenerator;
};

} // namespace fw
} // namespace nfd

#endif // BIGDATA_RANDOM_STRATEGY_HPP
