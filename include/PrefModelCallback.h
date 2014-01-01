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

#ifndef _I_PREF_MODEL_CALLBACK__
#define _I_PREF_MODEL_CALLBACK__

#include <DealStateType.h>
#include <PrefTypes.h>

namespace Preference {

class PrefModelCallback {
public:
	virtual ~PrefModelCallback() = 0;

	// Model events processor. Order of callback methods depends on a game state
	// except ProcessModelChanged - it is invoked in the end of atomic model update operations.
	void ProcessNewLayoutStart();
	void ProcessDealStateChanged(DealStateType prevState, DealStateType newState);
	void ProcessPlayerBid(BidType);
	void ProcessPlayerDrop(CardsSet drop, BidType deal);
	void ProcessPlayerMove(Card);
	void ProcessHandOpening(CardsSet set, int player);
	void ProcessPassoutWidowCardOpening(Card card);
	void ProcessWidowOpening(CardsSet widow);
	// Method invoked when model has processed update event.
	// No callback methods invoked before model update method finished
	void ProcessModelChanged();

protected:
	virtual void processNewLayoutStart() = 0;
	virtual void processDealStateChanged(DealStateType prevState, DealStateType newState) = 0;
	virtual void processPlayerBid(BidType) = 0;
	virtual void processPlayerDrop(CardsSet drop, BidType deal) = 0;
	virtual void processPlayerMove(Card) = 0;
	virtual void processHandOpening(CardsSet set, int player) = 0;
	virtual void processPassoutWidowCardOpening(Card card) = 0;
	virtual void processWidowOpening(CardsSet widow) = 0;
	virtual void processModelChanged() = 0;
};

inline PrefModelCallback::~PrefModelCallback() {}

inline void PrefModelCallback::ProcessNewLayoutStart()
{
	if( this != 0 ) {
		processNewLayoutStart();
	}
}
	
inline void PrefModelCallback::ProcessDealStateChanged(DealStateType prevState, DealStateType newState)
{
	if( this != 0 ) {
		processDealStateChanged(prevState, newState);
	}
}
	
inline void PrefModelCallback::ProcessPlayerBid(BidType bid)
{
	if( this != 0 ) {
		processPlayerBid(bid);
	}
}

inline void PrefModelCallback::ProcessPlayerDrop(CardsSet drop, BidType deal)
{
	if( this != 0 ) {
		processPlayerDrop(drop, deal);
	}
}

inline void PrefModelCallback::ProcessPlayerMove(Card move)
{
	if( this != 0 ) {
		processPlayerMove(move);
	}
}

inline void PrefModelCallback::ProcessHandOpening(CardsSet set, int player)
{
	if( this != 0 ) {
		processHandOpening(set, player);
	}
}

inline void PrefModelCallback::ProcessPassoutWidowCardOpening(Card card)
{
	if( this != 0 ) {
		processPassoutWidowCardOpening(card);
	}
}

inline void PrefModelCallback::ProcessWidowOpening(CardsSet widow)
{
	if( this != 0 ) {
		processWidowOpening(widow);
	}
}

inline void PrefModelCallback::ProcessModelChanged()
{
	if( this != 0 ) {
		processModelChanged();
	}
}

}
#endif // _I_PREF_MODEL_CALLBACK__
