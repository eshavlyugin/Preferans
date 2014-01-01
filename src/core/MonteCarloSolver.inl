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

#ifndef _MONTE_CARLO_SOLVER_INL__
#define _MONTE_CARLO_SOLVER_INL__

// constant using in UCB formula
static const float C = 5.0f;

//-------------------------------MonteCarloSolver inline methods--------------------------------
//
template<class POLICY>
inline MonteCarloSolver<POLICY>::MonteCarloSolver(POLICY& _policy) : tree(0), policy(_policy), playoutsCount(0)
{
}

template<class POLICY>
inline MonteCarloSolver<POLICY>::~MonteCarloSolver() 
{
	if( tree != 0 ) {
		delete tree;
	}
}

template<class POLICY>
float MonteCarloSolver<POLICY>::PlayerScoreEstimation(int player)
{
	assert(tree != 0);
	return tree->TotalScores[player];
}

template<class POLICY>
void MonteCarloSolver<POLICY>::Solve(const Layout& layout, int totalSimulationsCount) 
{
	tree = new McTreeNode(0);
	playoutsCount = 0;
	while( playoutsCount < totalSimulationsCount ) {
		Layout root = layout;
		path.clear();
		// start simulation
		processNode(tree, root);
	}
}

template<class POLICY>
bool MonteCarloSolver<POLICY>::isAcceptableMove(const Layout& layout, Card move)const
{
	if( layout.MoveSuit != GetCardSuit(move) && layout.MoveSuit != SuitNoTrump 
		&& HasCardsOfSuit(layout.Cards[layout.CurrentPlayer], layout.MoveSuit ) ) 
	{
		return false;
	}
	if( !IsSetContainsCard(layout.Cards[layout.CurrentPlayer], move) ) {
		return false;
	}
	// TODO: cut-off useless moves like sibbling
	return true;
}

template<class POLICY>
void MonteCarloSolver<POLICY>::processUnexpandedNode(McTreeNode* node, Layout& layout)
{
	// We have reached unexpanded node. Switching to random simulation
	McTreeNode* lastChild = 0;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
			Card card = CreateCard( itSuit.GetObject().Value, itRank.GetObject().Value );
			// If move would never been played in real game, ignore it
			if( !isAcceptableMove( layout, card ) ) {
				continue;
			}
		
			if( lastChild == 0 ) {
				node->FirstChild = new McTreeNode(node);
				path.push_back(node->FirstChild);
				lastChild = node->FirstChild;
			} else {
				lastChild->Next = new McTreeNode(node);
				lastChild = lastChild->Next;
				path.back() = lastChild;
			}

			lastChild->Move = card;

			Layout tmp = layout;
			tmp.UpdateOnMove(card);
			playRestOfGame(tmp);
		}
	}
}

template<class POLICY>
void MonteCarloSolver<POLICY>::processExpandedNode(McTreeNode* node, Layout& layout)
{
	// Select next move using normal distribution law
	McTreeNode* child = node->FirstChild;
	McTreeNode* best = child;
	float quality = -1000.0f;
	int currentPlayer = layout.CurrentPlayer;
	while( child != 0 ) {
		float tmp = - child->TotalScores[currentPlayer] / child->PlayoutsCount + 
			C * sqrt(log(static_cast<float>(playoutsCount)) / child->PlayoutsCount);
		if( tmp > quality ) {
			quality = tmp;
			best = child;
		}
		child = child->Next;
	}
	layout.UpdateOnMove(best->Move);
	processNode(best, layout);
}


template<class POLICY>
void MonteCarloSolver<POLICY>::processNode(McTreeNode* node, Layout& layout)
{
	path.push_back(node);

	McTreeNode* child = node->FirstChild;
	if( layout.IsFinished() ) {
		playRestOfGame(layout);
	} if( child == 0 ) {
		processUnexpandedNode( node, layout );
	} else {
		processExpandedNode( node, layout );
	}
}

template<class POLICY>
void MonteCarloSolver<POLICY>::playRestOfGame(Layout& layout)
{
	while( !layout.IsFinished() ) {
		Card move = policy.NextMove(layout);
		layout.UpdateOnMove(move);
	}

	playoutsCount++;
	// updating statistics in tree
	for( int i = 0; i < path.size(); i++ ) {
		path[i]->PlayoutsCount++;
		for( int j = 0; j < NumOfPlayers; j++ ) {
			path[i]->TotalScores[j] += layout.Tricks[j];
		}
	}
}

#endif // _MONTE_CARLO_SOLVER_INL__

