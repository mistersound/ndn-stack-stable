#ifndef BIGDATA_DEFAULT_STRATEGY_HPP
#define BIGDATA_DEFAULT_STRATEGY_HPP

#include <boost/random/mersenne_twister.hpp>
#include "face/face.hpp"
#include "fw/best-route-strategy.hpp"
#include "fw/retx-suppression-exponential.hpp"

namespace nfd {
namespace fw {

class BigDataDefaultStrategy: public BestRouteStrategy {
public:
	BigDataDefaultStrategy(Forwarder& forwarder, const Name& name = STRATEGY_NAME);

	virtual ~BigDataDefaultStrategy();

	virtual void
	afterReceiveInterest(const Face& inFace, const Interest& interest,
			shared_ptr<fib::Entry> fibEntry, shared_ptr<pit::Entry> pitEntry);

public:
	static const Name STRATEGY_NAME;

protected:
	bool m_replicated = false;
	RetxSuppressionExponential m_retxSuppression;
};

} // namespace fw
} // namespace nfd

#endif // BIGDATA_STRATEGY_HPP
