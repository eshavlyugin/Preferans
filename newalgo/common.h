#pragma once

#include <bits/stdc++.h>

#define PREF_ASSERT(EXPR) PrefAssertImpl(EXPR, #EXPR , __FILE__, __LINE__ )

using namespace std;

typedef void AssertHandler(const char* assert, const char* file, long line);

extern AssertHandler* globalAssertHandler;

static void SetAssertHandler(AssertHandler* handler) {
	globalAssertHandler = handler;
}

static void PrefAssertImpl(bool passed, const char* assert, const char* file,
		long line)
{
	if (!passed) {
		globalAssertHandler(assert, file, line);
	}
}

class CardsSetIterator;

enum Suit
	: uint8_t {
		Spades = 0, Clubs, Diamonds, Hearts, NoSuit
};

using Rank = uint8_t;

struct Card;

inline std::string CardToString(Card c);
inline Card StringToCard(const string& s);

struct Card {
	Card() { val_ = 0; }
	explicit Card(uint32_t val) { val_ = val; }
	explicit Card(const std::string& s) { this->val_ = StringToCard(s).val_; }

	//operator uint32_t() const { return val_; }
	operator std::string() const { return CardToString(*this); }

	bool operator == (Card other) const {
		return val_ == other.val_;
	}
	bool operator != (Card other) const {
		return val_ != other.val_;
	}
	bool operator > (Card other) const {
		return val_ > other.val_;
	}
	bool operator < (Card other) const {
		return val_ < other.val_;
	}

	uint32_t val_ = 0;
};

static const Card NoCard(0);

class CardsSetIterator : public std::iterator<input_iterator_tag, Card> {
public:
	CardsSetIterator(uint32_t cards) :
		cards_(cards)
	{
		Shift();
	}

	CardsSetIterator& operator++() {
		Shift();
		return *this;
	}
	CardsSetIterator operator++(int) {
		CardsSetIterator orig(*this);
		Shift();
		return orig;
	}

	Card* operator->() {
		return &card_;
	}
	Card& operator*() {
		return card_;
	}

	bool operator!=(const CardsSetIterator& other) const {
		return curShift_ != other.curShift_;
	}
	bool operator==(const CardsSetIterator& other) const {
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
			card_ = Card(1u << (curShift_ - 1));
		}
	}

private:
	Card card_;
	uint32_t cards_ = 0;
	uint32_t curShift_ = 0;
};

class CardsSet {
public:
	using iterator = CardsSetIterator;

public:
	CardsSet() :
			cards_(0) {
	}

	explicit CardsSet(uint32_t cards) :
			cards_(cards) {
	}

	explicit CardsSet(Card c)
		: cards_(c.val_)
	{
	}

	operator uint32_t() const {
		return cards_;
	}

	uint32_t Size() const {
		return __builtin_popcount(cards_);
	}

	CardsSet GetSubsetOfSuit(Suit s) const {
		return CardsSet(cards_ & (0xff << (s * 8)));
	}

	bool IsInSet(Card c) const {
		return (cards_ & c.val_) != 0;
	}

	CardsSet& Remove(Card c) {
		cards_ &= ~c.val_;
		return *this;
	}

	CardsSet& Add(CardsSet s) {
		cards_ |= s.cards_;
		return *this;
	}

	CardsSet& Add(Card c) {
		cards_ |= c.val_;
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

inline char SuitToChar(Suit s) {
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
	    PREF_ASSERT(false && "Unknown card suit");
		return ' ';
	}
}

inline Suit CharToSuit(char c) {
    switch (c) {
    case 's':
        return Spades;
    case 'c':
        return Clubs;
    case 'd':
        return Diamonds;
    case 'h':
        return Hearts;
    case 'n':
        return NoSuit;
    default:
        PREF_ASSERT(false && "Unknown card suit");
        return NoSuit;
    }
}

inline char RankToChar(Rank r) {
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
	    PREF_ASSERT(false && "Unknown card rank");
		return ' ';
	}
}

inline Rank CharToRank(char c) {
    switch (c) {
    case '7':
        return 0;
    case '8':
        return 1;
    case '9':
        return 2;
    case 'T':
        return 3;
    case 'J':
        return 4;
    case 'Q':
        return 5;
    case 'K':
        return 6;
    case 'A':
        return 7;
    default:
        PREF_ASSERT(false && "Unknown card rank");
        return 0xff;
    }
}
inline bool IsValidCard(Card c) {
	return __builtin_popcount(c.val_) == 1;
}

inline Card MakeCard(Suit s, Rank r) {
	return Card(1u << (s * 8 + r));
}

inline Rank GetRank(Card c) {
	return __builtin_ctz(c.val_) & 7;
}

inline Suit GetSuit(Card c) {
	return (Suit) (__builtin_ctz(c.val_) >> 3);
}

inline uint8_t GetCardBit(Card c) {
	return __builtin_ctz(c.val_);
}

inline Card CardFromBit(uint8_t bit) {
	return Card(1 << bit);
}

inline string CardToString(Card c) {
	return {RankToChar(GetRank(c)), SuitToChar(GetSuit(c))};
}

inline Card StringToCard(const string& s) {
    PREF_ASSERT(s.size() == 2);
    auto rank = CharToRank(s[0]);
    auto suit = CharToSuit(s[1]);
    return MakeCard(suit, rank);
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
		PREF_ASSERT(IsValidMove(c));
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
			fill(ondesk.begin(), ondesk.end(), NoCard);
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
			if (card == NoCard) {
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
		ost << "Trump: " << SuitToChar(trump) << "\n";
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
	array<Card, 3> ondesk = {{NoCard, NoCard, NoCard}};
	array<uint8_t, 3> scores = { { 0 } };
	array<bool, 3> isClosed = { { false } };
	array<array<bool, 4>, 3> playerSuitOut_ = { { { { false } } } };
	Suit trump = NoSuit;
	uint8_t moveNumber = 0;
	uint8_t first = 0;
	uint8_t turn = 0;
	uint8_t curBest = 0;
};

inline istream& operator >> (istream& ist, GameState& gs) {
    array<CardsSet, 3> hands;
    for (auto& set : hands) {
        for (uint32_t i = 0; i < 10; ++i) {
            string tmp;
            ist >> tmp;
            set.Add(StringToCard(tmp));
        }
    }
    uint32_t first;
    ist >> first;
    gs = GameState(hands, first, NoSuit);
    return ist;
}

inline ostream& operator << (ostream& ost, const GameState& gs) {
    for (uint32_t player = 0; player < 3; ++player) {
        for (auto card : gs.Hand(player)) {
            ost << CardToString(card) << " ";
        }
        ost << "\n";
    }
    ost << gs.GetFirstPlayer() << "\n";
    return ost;
}

using CardsProbabilities = array<array<float, 32>, 4>;
