#ifndef __I_PREF_ENGINE_H__
#define __I_PREF_ENGINE_H__

#include <PrefGameModel.h>
#include <PrefModelCallback.h>
#include <PrefEngineCallback.h>

namespace Preference {

class PrefEngine : public PrefModelCallback {
public:
	virtual ~PrefEngine() = 0;

	// Move methods
	virtual Card DoMove() = 0;
	virtual BidType DoBid() = 0;
	virtual Drop DoDrop() = 0;
	// Offer
	virtual bool AcceptOffer(int offer) = 0;
	// Set game model. Use model methods to get information about current game state
	virtual void SetModel(const PrefGameModel* model) = 0;
	// Set player number in model
	virtual void SetNumInModel(int) = 0;
	// Set engine callback
	virtual void SetEngineCallback(PrefEngineCallback* callback) = 0;
	
protected:
	// PrefModelCallback
	virtual void processNewLayoutStart() = 0;
	virtual void processDealStateChanged(DealStateType prevState, DealStateType newState) = 0;
	virtual void processPlayerBid(BidType) = 0;
	virtual void processPlayerDrop(CardsSet drop, BidType deal) = 0;
	virtual void processPlayerMove(Card) = 0;
};

inline PrefEngine::~PrefEngine()
{
}

}

#endif // __I_PREF_ENGINE_H__

