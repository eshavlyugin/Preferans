/****************************************************************************
 Preferans: implementation of card-tricking game Preferans (or Preference).
 ****************************************************************************
 Copyright (c) 2010-2011  Eugene Shavlyugin <eshavlyugin@gmail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ****************************************************************************/

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <GameSettings.h>
#include <EngineSettings.h>
#include <PrefEngine.h>
#include <PrefEngineCallback.h>

class EngineData;
class DealState;
class Bullet;
class RandomLayoutsGenerator;
struct MoveQuality;

//---------------PrefEngineImpl-----------------------------------------
//
class PrefEngineImpl : public PrefEngine {
public:
	PrefEngineImpl(const string& dataPath);
	~PrefEngineImpl();

	void SetEngineSettings(const EngineSettings& _engineSettings ) { engineSettings = _engineSettings; }
	void SetCallback(PrefEngineCallback* _callback) { callback = _callback; }

	// PrefPrefEngineImpl
	virtual Card DoMove();
	virtual BidType DoBid();
	virtual Drop DoDrop();
	virtual bool AcceptOffer(int howMany);
	virtual void SetModel(const PrefGameModel* _model) { model = _model; }
	virtual void SetNumInModel(int num) { playerNum = num; }
	virtual void SetEngineCallback(PrefEngineCallback*);

protected:
	// PrefEngine::PrefModelCallback
	virtual void processNewLayoutStart();
	virtual void processDealStateChanged(DealStateType prevState, DealStateType newState);
	virtual void processPlayerBid(BidType);
	virtual void processPlayerDrop(CardsSet drop, BidType deal);
	virtual void processPlayerMove(Card);
	virtual void processHandOpening(CardsSet set, int player);
	virtual void processPassoutWidowCardOpening(Card card);
	virtual void processWidowOpening(CardsSet widow);
	virtual void processModelChanged() {}

private:
	struct CandidateDrop {
		CardsSet Cards;
		float Weight;

		CandidateDrop(CardsSet cards, float weight) : Cards(cards), Weight(weight) {}
	};

	PrefEngineCallback* callback;
	// Number of our player in model
	int playerNum;
	// Greater rank means more probability for layout to be chosen
	// layoutsRanks[suit][player][layoutMask] 
	scoped_ptr<RandomLayoutsGenerator> layoutsGenerator;
	// outcomes for trials. used for bidding. (i,j) - taken i where suit j
	mutable multi_array<int, 2> contractTrialsOutcomes;
	// Layout Database
	scoped_ptr<EngineData> data;
	// game model
	const PrefGameModel* model;
	// engine settings
	EngineSettings engineSettings;

	// Suit sets ranks updating methods
	void updateRanksAfterBid(BidType);
	void updateRanksAfterDrop(const Drop& drop);
	void updateRanksAfterMove(Card move);	
	void updateRanksAfterWidowOpened(CardsSet widow);
	void updateRanksAfterPassoutWidowOpened(Card card);

	void forbidCardsSet(CardsSet set, int player, Suit suit);
	void forbidAllSetsWithoutCardsSet(CardsSet set, int player, Suit suit);
	
	// Playing
	Card doPassoutMove();
	Card doMisereMove();
	Card doContractGameMove();
	// Bidding
	BidType doBid();
	BidType doTradeBid();
	BidType doWhistBid();
	BidType doOpenOrCloseWhistBid();
	// Dropping
	Drop doDrop() const;
	Drop doMisereDrop(CardsSet hand, const vector< vector<Card> >& candidateDrops ) const;
	Drop doContractGameDrop(CardsSet hand, const vector< vector<Card> >& candidateDrops ) const;
	void filterDropsByEmptyHands( vector< vector<Card> >& drops, CardsSet dealer, CardsSet utilizedCards) const;

	bool isPlayMisereGoodIdea() const;
	BidType getMaxAcceptableContractGame() const;
	Suit determineBestTrump(CardsSet set) const;
	vector<float> getGamesEvaluations(int deal) const;
	void selectBestMoves(const vector<MoveQuality>& qualities, vector<Card>& result, bool isMax) const;
	BidType determineBestContract(CardsSet set) const;
	Drop selectDropFromCandidates( const vector<CandidateDrop>& candidates ) const;
	BidType chooseContractDeal(CardsSet drop) const;
	void prepareLayout( Layout& layout ) const;
	void prepareContractGameLayout(Layout& layout) const;
	void prepareMisereLayout(Layout& layout) const;
};

#endif // _ENGINE_H_

