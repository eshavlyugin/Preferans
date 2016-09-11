#pragma once

#include <bits/stdc++.h>

#undef assert

using namespace std;

static void my_assert(bool passed, const char* assert, const char* file,
		long line) {
	if (!passed) {
		cerr << "failed assert " << assert << " in " << file << " at " << line
				<< ".\n";
		abort();
	}
}

#define assert(EXPR) my_assert(EXPR, #EXPR , __FILE__, __LINE__ )

class CardsSetIterator;

enum Suit
	: uint8_t {
		Spades = 0, Clubs, Diamonds, Hearts, NoSuit
};

using Rank = uint8_t;
using Card = uint32_t;

static const Card NoCard = 0;

class CardsSetIterator {
public:
	CardsSetIterator(uint32_t cards) :
			cards_(cards) {
		Shift();
	}

	CardsSetIterator& operator++() {
		Shift();
		return *this;
	}

	Card operator*() const {
		return 1u << (curShift_ - 1);
	}

	bool operator !=(const CardsSetIterator& other) {
		return curShift_ != other.curShift_;
	}
	bool operator ==(const CardsSetIterator& other) {
		return curShift_ == other.curShift_;
	}

private:
	void Shift() {
		if (cards_ == 0) {
			curShift_ = 64;
		} else {
			auto pos = __builtin_ctz(cards_);
			curShift_ += pos + 1;
			cards_ >>= pos + 1;
		}
	}

private:
	uint32_t cards_;
	uint32_t curShift_ = 0;
};

class CardsSet {
public:
	CardsSet() :
			cards_(0) {
	}

	CardsSet(uint32_t cards) :
			cards_(cards) {
	}

	operator uint32_t() const {
		return cards_;
	}

	uint32_t Size() const {
		return __builtin_popcount(cards_);
	}

	CardsSet GetSubsetOfSuit(Suit s) const {
		return cards_ & (0xff << (s * 8));
	}

	bool IsInSet(Card c) const {
		return (cards_ & c) != 0;
	}

	CardsSet& Remove(Card c) {
		cards_ &= ~c;
		return *this;
	}

	CardsSet& Add(CardsSet& s) {
		cards_ |= s.cards_;
		return *this;
	}

	CardsSet& Add(Card c) {
		cards_ |= c;
		return *this;
	}

	bool Empty() const {
		return cards_ == 0;
	}

	CardsSetIterator begin() const {
		return CardsSetIterator(cards_);
	}

	CardsSetIterator end() const {
		return CardsSetIterator(0);
	}

private:
	uint32_t cards_;
};

inline char SuitToString(Suit s) {
	switch (s) {
	case Spades:
		return 's';
	case Clubs:
		return 'c';
	case Diamonds:
		return 'd';
	case Hearts:
		return 'h';
	case NoSuit:
		return 'n';
	default:
		return ' ';
	}
}

inline char RankToString(Rank r) {
	switch (r) {
	case 0:
		return '7';
	case 1:
		return '8';
	case 2:
		return '9';
	case 3:
		return 'T';
	case 4:
		return 'J';
	case 5:
		return 'Q';
	case 6:
		return 'K';
	case 7:
		return 'A';
	default:
		return ' ';
	}
}

inline Card IsValidCard(Card c) {
	return c != 0;
}

inline Card MakeCard(Suit s, Rank r) {
	return 1u << (s * 8 + r);
}

inline Rank GetRank(Card c) {
	return __builtin_ctz(c) & 7;
}

inline Suit GetSuit(Card c) {
	return (Suit) (__builtin_ctz(c) >> 3);
}

inline uint8_t GetCardBit(Card c) {
	return __builtin_ctz(c);
}

inline Card CardFromBit(uint8_t bit) {
	return Card(1 << bit);
}

inline string CardToString(Card c) {
	return {RankToString(GetRank(c)), SuitToString(GetSuit(c))};
}

inline bool IsCardCovers(Card c1, Card c2, Suit trump) {
	if (GetSuit(c1) != GetSuit(c2)) {
		return GetSuit(c1) == trump;
	} else {
		return GetRank(c1) > GetRank(c2);
	}
}

class GameState {
public:
	struct MoveData {
		Card card_;
		uint8_t player_;
	};

public:
	GameState() {
	}
	GameState(array<CardsSet, 3> a, uint8_t first, Suit trump) {
		hands = a;
		this->trump = trump;
		this->first = first;
		turn = first;
		fill(scores.begin(), scores.end(), 0);
	}

	uint32_t PlayerWithGreaterCard() const {
		return turn == first ? 3 : curBest;
	}

	uint32_t GetFirstPlayer() const {
		return first;
	}

	CardsSet GenValidMoves() const {
		if (turn == first) {
			return hands[first];
		} else {
			Suit s = GetSuit(ondesk[first]);
			auto cardsOfSuit = hands[turn].GetSubsetOfSuit(s);
			if (!cardsOfSuit.Empty()) {
				return cardsOfSuit;
			}
			if (trump != NoSuit) {
				cardsOfSuit = hands[turn].GetSubsetOfSuit(trump);
				if (!cardsOfSuit.Empty()) {
					return cardsOfSuit;
				}
			}
			return hands[turn];
		}
	}

	CardsSet Hand(uint32_t player) const {
		return hands[player];
	}

	CardsSet Out() const {
		return out;
	}

	void MakeMove(Card c) {
		assert(IsValidMove(c));
		history_.push_back( { c, turn });
		hands[turn].Remove(c);
		out.Add(c);
		ondesk[turn] = c;
		if (turn != first && GetSuit(c) != GetSuit(ondesk[first])) {
			playerSuitOut_[turn][(uint8_t) GetSuit(ondesk[first])] = true;
		}

		if (turn == first || IsCardCovers(c, ondesk[curBest], trump)) {
			curBest = turn;
		}
		turn++;
		if (turn >= 3) {
			turn = 0;
		}

		if (turn == first) {
			first = curBest;
			turn = first;
			scores[curBest]++;
			fill(ondesk.begin(), ondesk.end(), 0);
			moveNumber++;
		}
	}

	uint8_t GetCurPlayer() const {
		return turn;
	}

	const array<uint8_t, 3>& GetScores() const {
		return scores;
	}

	Suit GetTrump() const {
		return trump;
	}

	Card OnDesk(uint32_t player) const {
		return ondesk[player];
	}

	void Dump(ostream& ost) const {
		for (auto cardsSet : hands) {
			ost << "Hand: ";
			for (auto card : cardsSet) {
				ost << CardToString(card) << " ";
			}
			ost << "\n";
		}

		ost << "InGame: ";
		for (auto card : ondesk) {
			if (card == 0) {
				ost << "-- ";
			} else {
				ost << CardToString(card) << " ";
			}
		}
		ost << "\n";
		ost << "Scores: ";
		for (auto score : scores) {
			ost << (uint32_t) score << " ";
		}
		ost << "\n";
		ost << "Move number: " << (uint32_t) moveNumber << "\n";
		ost << "First hand: " << (uint32_t) first << "\n";
		ost << "Current hand: " << (uint32_t) turn << "\n";
		ost << "Trump: " << SuitToString(trump) << "\n";
	}

	bool IsHandClosed(uint32_t hand) const {
		return isClosed[hand];
	}

	void CloseHands(const vector<uint8_t>& toClose) {
		for (auto hand : toClose) {
			hands[hand] = CardsSet(0);
			isClosed[hand] = true;
		}
	}

	const vector<MoveData>& GetMoveHistory() const {
		return history_;
	}

	bool IsSuitOut(uint8_t player, Suit suit) const {
		return playerSuitOut_[player][(uint8_t) suit];
	}

	void OpenHand(CardsSet Hand) {
	}

	bool IsFinished() const {
		return all_of(hands.begin(), hands.end(),
				[](auto x) {return x.Empty();});
	}

	CardsSet GetKnownCards() const {
		CardsSet result = Out();
		for (uint32_t i = 0; i < 3; i++) {
			result.Add(Hand(i));
		}
		return result;
	}

	uint8_t GetMoveNumber() const {
		return moveNumber;
	}

	bool IsValidMove(Card c) const {
		if (isClosed[turn]) {
			return true;
		}
		return GenValidMoves().IsInSet(c);
	}

private:
	vector<MoveData> history_;
	array<CardsSet, 3> hands;
	CardsSet out;
	array<Card, 3> ondesk = { { 0 } };
	array<uint8_t, 3> scores = { { 0 } };
	array<bool, 3> isClosed = { { false } };
	array<array<bool, 4>, 3> playerSuitOut_ = { { { { false } } } };
	Suit trump = NoSuit;
	uint8_t moveNumber = 0;
	uint8_t first = 0;
	uint8_t turn = 0;
	uint8_t curBest = 0;
};

using CardsProbabilities = array<array<float, 32>, 4>;
