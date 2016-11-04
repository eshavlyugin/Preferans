#pragma once

#include "common.h"

#include <bits/stdc++.h>

enum FeatureTag {
	FT_Playing,
	FT_PosPredict,
	FT_TypeCount
};

class FeaturesSet;
class FeaturesRange;
class FeaturesRegistry;

class FeaturesSet {
	friend class FeaturesRegistry;

public:
	void Set(const FeaturesRange& range, uint32_t pos, float value);
	void SetUT(uint32_t pos, float value);
	vector<float> GetFeatures() const;
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
	vector<FeaturesRange*> GetRangesInOrder() const;
	FeaturesSet CreateEmptySet();
	uint32_t GetTotal() const {
		return total_;
	}

private:
	void RegisterRange(FeaturesRange* range);
};

struct PlayFeaturesRegistry : public FeaturesRegistry {
	PlayFeaturesRegistry();

	FeaturesRange PlayerCards[1];
	FeaturesRange NotInGameCards;
	FeaturesRange CardsOnDesk[2];
	FeaturesRange IsGreaterCard;
	FeaturesRange IsValidMove;
};

struct PredictPosFeaturesRegistry : public FeaturesRegistry {
	PredictPosFeaturesRegistry();

	FeaturesRange Move;
	FeaturesRange CardsOnDesk[2];
	FeaturesRange NotInGameCards;
};

string TagToString(FeatureTag tag);
FeatureTag StringToTag(const string& tagName);

FeaturesSet CalcFeatures(const GameState& playerView, Card move, uint32_t ourHero, FeatureTag tag);
uint32_t EncodeMoveIndex(const GameState& playerView, Card c);
