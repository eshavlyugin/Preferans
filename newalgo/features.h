#pragma once

#include "common.h"

#include <bits/stdc++.h>

enum FeatureTag {
	FT_Move,
	FT_CommonCards,
	FT_OpenCards,
	FT_CloseCards,
	FT_TypeCount
};

struct Feature {
	float value_ = 0.0f;
	FeatureTag tag_ = FT_CommonCards;
};

class FeaturesSet;
class FeaturesRange;
class FeaturesRegistry;

class FeaturesSet {
	friend class FeaturesRegistry;

public:
	void Set(const FeaturesRange& range, uint32_t pos, float value);
	void SetUT(uint32_t pos, float value);
	vector<Feature> GetFeatures() const;
	vector<pair<uint32_t, float>> GetNonZeroFeatures() const;

private:
	// Create through FeaturesRegistry
	FeaturesSet(FeaturesRegistry* registry);

private:
	vector<pair<uint32_t, float>> features_;
	FeaturesRegistry* registry_;
};

class FeaturesRange {
	friend class FeaturesRegistry;

public:
	FeaturesRange(FeaturesRegistry* registry, uint32_t length, FeatureTag type);

	uint32_t GetStart() const {
		return start_;
	}
	uint32_t GetLength() const {
		return length_;
	}
	FeatureTag GetType() const {
		return type_;
	}

private:
	uint32_t start_;
	uint32_t length_;
	FeatureTag type_;
};

class FeaturesRegistry {
	friend class FeaturesRange;
	friend class FeaturesSet;

private:
	uint32_t total_ = 0;
	vector<FeaturesRange*> rangesInOrder_;

public:
	FeaturesRegistry();

	FeaturesRange PlayerCards[3];
	FeaturesRange NotInGameCards;
	FeaturesRange CardsOnDesk[3];
	FeaturesRange Trump;
	FeaturesRange Move;
	FeaturesRange NumOfSuit;
	FeaturesRange LastMove1[3];
	FeaturesRange LastMove2[3];
	FeaturesRange RealHands[3];
	FeaturesRange RealOutOfGame;
	FeaturesRange CurrentMove;
	FeaturesRange PlayerWithGreaterCard;
	FeaturesRange MoveNumber;
	FeaturesRange FirstPlayer;
	FeaturesRange IsGreaterCard;

	vector<FeaturesRange*> GetRangesInOrder() const;
	FeaturesSet CreateEmptySet();
	uint32_t GetTotal() const {
		return total_;
	}

private:
	void RegisterRange(FeaturesRange* range);
};

FeaturesSet CalcFeatures(const GameState& playerView, const GameState& realState,
		const CardsProbabilities& probArray, Card move, uint32_t ourHero);
