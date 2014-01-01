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

#ifndef _MONTE_CARLO_SOLVER_H__
#define _MONTE_CARLO_SOLVER_H__

/*
 Stochastic model fits better for passout game. So monte carlo tree search is used
 */

//--------------------------------------Monte carlo tree node----------------------------------------------------

struct McTreeNode {
	// First child node of tree
	McTreeNode* FirstChild;
	// Next child after current node
	McTreeNode* Next;
	// Parent node
	McTreeNode* Parent;
	// Number of games played from this node
	int PlayoutsCount;
	// Sum of scores for all playouts from this node for all players
	vector<int> TotalScores;
	// Only for non-root nodes: move from parent node leading to position
	Card Move;

	McTreeNode(McTreeNode* parent);
	~McTreeNode();
};

inline McTreeNode::McTreeNode(McTreeNode* parent) : 
	FirstChild(0), 
	Next(0), 
	Parent(parent), 
	PlayoutsCount(0), 
	TotalScores(3, 0),
	Move(UnknownCard)
{
}

inline McTreeNode::~McTreeNode()
{
	if( FirstChild != 0 ) {
		delete FirstChild;
		FirstChild = 0;
	}
	if( Next != 0 ) {
		delete Next;
		Next = 0;
	}
}

//-----------------------------------------Passout solver--------------------------------------

template<class POLICY> class MonteCarloSolver {
public:
	MonteCarloSolver(POLICY& policy);
	~MonteCarloSolver();

	void Solve(const Layout&, int totalSimulationsCount);
	float PlayerScoreEstimation(int player);
	const McTreeNode* ResultTreeRoot() const { return tree; }

private:
	POLICY& policy;
	McTreeNode* tree;
	int playoutsCount;
	int totalSimulationsCount;
	vector<McTreeNode*> path;

	bool isAcceptableMove(const Layout& layout, Card move) const;
	void processExpandedNode(McTreeNode* currentNode, Layout& layout);
	void processUnexpandedNode(McTreeNode* currentNode, Layout& layout);
	void processNode(McTreeNode* currentNode, Layout& layout);
	void playRestOfGame(Layout& layout);
};

#include <MonteCarloSolver.inl>

#endif // _MONTE_CARLO_SOLVER_H__

