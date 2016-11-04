#include "features.h"

using namespace std;

constexpr uint32_t NumCardFeatures = 32;

FeaturesSet::FeaturesSet(FeaturesRegistry* registry) :
		registry_(registry) {
}

void FeaturesSet::Set(const FeaturesRange& range, uint32_t pos, float value) {
	PREF_ASSERT(range.GetLength() > pos);
	auto finalPos = range.GetStart() + pos;
	features_.push_back(make_pair(finalPos, value));
}

void FeaturesSet::SetUT(uint32_t pos, float value) {
	features_.push_back(make_pair(pos, value));
}

vector<float> FeaturesSet::GetFeatures() const {
	auto ranges = registry_->GetRangesInOrder();
	vector<float> result(registry_->GetTotal());
	for (auto pair : features_) {
		result[pair.first] = pair.second;
	}
	return result;
}

vector<pair<uint32_t, float>> FeaturesSet::GetNonZeroFeatures() const {
	return features_;
}

std::vector<FeaturesRange*> FeaturesRegistry::GetRangesInOrder() const {
	return rangesInOrder_;
}

FeaturesSet FeaturesRegistry::CreateEmptySet() {
	return FeaturesSet(this);
}

void FeaturesRegistry::RegisterRange(FeaturesRange* range) {
	rangesInOrder_.push_back(range);
	total_ += range->GetLength();
}

PlayFeaturesRegistry::PlayFeaturesRegistry()
	: PlayerCards({ FeaturesRange(this, NumCardFeatures, FT_Playing)})
	, NotInGameCards(this, NumCardFeatures, FT_Playing)
	, CardsOnDesk({ FeaturesRange(this, NumCardFeatures, FT_Playing), FeaturesRange(this, NumCardFeatures, FT_Playing) })
	, IsGreaterCard(this, 32, FT_Playing)
	, IsValidMove(this, 32, FT_Playing)
{
}

PredictPosFeaturesRegistry::PredictPosFeaturesRegistry()
	: Move(FeaturesRange(this, NumCardFeatures, FT_PosPredict))
	, CardsOnDesk({ FeaturesRange(this, NumCardFeatures, FT_PosPredict), FeaturesRange(this, NumCardFeatures, FT_PosPredict)})
	, NotInGameCards(FeaturesRange(this, NumCardFeatures, FT_PosPredict))
{}

FeaturesRange::FeaturesRange(FeaturesRegistry* registry, uint32_t length,
		FeatureTag type) :
		start_(registry->GetTotal()), length_(length), type_(type) {
	registry->RegisterRange(this);
}

string TagToString(FeatureTag tag) {
	switch (tag) {
	case FeatureTag::FT_Playing:
		return "playing";
	case FeatureTag::FT_PosPredict:
		return "pos_predict";
	default:
		PREF_ASSERT(false);
		return "";
	}
}

FeatureTag StringToTag(const string& tagName) {
	if (tagName == "playing") {
		return FT_Playing;
	} else if (tagName == "pos_predict") {
		return FT_PosPredict;
	} else {
		PREF_ASSERT(false);
		return FT_TypeCount;
	}
}

uint32_t EncodeMoveIndex(const GameState& playerView, Card c) {
	auto suit = GetSuit(c);
	uint32_t realRank = 0;
	for (uint32_t rank = 0; rank < GetRank(c); rank++) {
		bool onDesk = false;
		for (uint32_t player = 0; player < 3; player++) {
			onDesk = onDesk || playerView.OnDesk(player) == c;
		}
		if (!playerView.Out().IsInSet(c) || onDesk) {
			realRank++;
		}
	}
	return GetCardBit(MakeCard(suit, realRank));
}

static FeaturesSet CalcPosPredictFeatures(const GameState& playerView, Card move, uint32_t ourHero) {
	static PredictPosFeaturesRegistry reg;

	FeaturesSet result = reg.CreateEmptySet();
	CardsSet knownCards = playerView.GetKnownCards();
	auto setCardF = [&](FeaturesRange& fr, Card c, float value) {
		result.Set(fr, GetCardBit(c), value);
	};

	for (Suit s = Spades; s != NoSuit; s = (Suit)(uint32_t(s) + 1)) {
		uint32_t realRank = 0;
		for (Rank r = 0; r < 8; ++r) {
			auto card = MakeCard(s, r);
			auto cardForFeature = card; //MakeCard(s, realRank);
			for (uint32_t i = 0, player = playerView.GetFirstPlayer(); i < 2; player = (player + 1) % 3, i++) {
				if (playerView.OnDesk(player) == card) {
					setCardF(reg.CardsOnDesk[i], cardForFeature, 1.0f);
				}
			}
			if (playerView.Out().IsInSet(card)) {
				setCardF(reg.NotInGameCards, cardForFeature, 1.0f);
			}
			if (move == card) {
				setCardF(reg.Move, cardForFeature, 1.0f);
			}
			realRank++;
		}
	}

	return result;
}

static FeaturesSet CalcPlayFeatures(const GameState& playerView, Card move, uint32_t ourHero) {
	static PlayFeaturesRegistry reg;

	FeaturesSet result = reg.CreateEmptySet();
	CardsSet knownCards = playerView.GetKnownCards();
	auto setCardF = [&](FeaturesRange& fr, Card c, float value) {
		result.Set(fr, GetCardBit(c), value);
	};

	for (Suit s = Spades; s != NoSuit; s = (Suit)(uint32_t(s) + 1)) {
		uint32_t realRank = 0;
		for (Rank r = 0; r < 8; ++r) {
			auto card = MakeCard(s, r);
			auto cardForFeature = card; //MakeCard(s, realRank);
			if (IsCardCovers(card, playerView.OnDesk(playerView.PlayerWithGreaterCard()), playerView.GetTrump())) {
				setCardF(reg.IsGreaterCard, cardForFeature, 1.0f);
			}
			setCardF(reg.IsValidMove, cardForFeature, playerView.IsValidMove(card) ? 1.0f : 0.0f);
			for (uint32_t i = 0, player = playerView.GetFirstPlayer(); i < 2; player = (player + 1) % 3, i++) {
				if (playerView.OnDesk(player) == card) {
					setCardF(reg.CardsOnDesk[i], cardForFeature, 1.0f);
				}
			}
			if (playerView.Hand(ourHero).IsInSet(card)) {
				setCardF(reg.PlayerCards[0], cardForFeature, 1.0f);
			}
			if (playerView.Out().IsInSet(card)) {
				setCardF(reg.NotInGameCards, cardForFeature, 1.0f);
			}
			realRank++;
		}
	}

	return result;
}

FeaturesSet CalcFeatures(const GameState& playerView, Card move, uint32_t ourHero, FeatureTag tag) {
	switch(tag) {
	case FT_Playing:
		return CalcPlayFeatures(playerView, move, ourHero);
	case FT_PosPredict:
		return CalcPosPredictFeatures(playerView, move, ourHero);
	default:
		{
			PREF_ASSERT(false);
			FeaturesRegistry reg;
			return reg.CreateEmptySet();
		}
	}
}
