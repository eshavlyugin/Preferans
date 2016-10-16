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

vector<Feature> FeaturesSet::GetFeatures() const {
	vector<Feature> result;
	auto ranges = registry_->GetRangesInOrder();
	for (auto range : ranges) {
		result.insert(result.end(), range->GetLength(),
				{ 0.0f, range->GetType() });
	}
	for (auto pair : features_) {
		result[pair.first].value_ = pair.second;
	}
	return result;
}

vector<pair<uint32_t, float>> FeaturesSet::GetNonZeroFeatures() const {
	return features_;
}

FeaturesRegistry::FeaturesRegistry()
		: PlayerCards({ FeaturesRange(this, NumCardFeatures, FT_CloseCards), FeaturesRange(this, NumCardFeatures, FT_CloseCards), FeaturesRange(this, NumCardFeatures, FT_CloseCards) })
		, NotInGameCards(this, NumCardFeatures, FT_CloseCards)
		, CardsOnDesk({ FeaturesRange(this, NumCardFeatures, FT_CommonCards), FeaturesRange(this, NumCardFeatures, FT_CommonCards), FeaturesRange(this, NumCardFeatures, FT_CommonCards) })
		, Move(this, NumCardFeatures, FT_Move)
		, IsGreaterCard(this, 32, FT_CommonCards)
{
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

FeaturesRange::FeaturesRange(FeaturesRegistry* registry, uint32_t length,
		FeatureTag type) :
		start_(registry->GetTotal()), length_(length), type_(type) {
	registry->RegisterRange(this);
}

string TagToString(FeatureTag tag) {
	switch (tag) {
	case FeatureTag::FT_Move:
		return "move";
	case FeatureTag::FT_CommonCards:
		return "common_cards";
	case FeatureTag::FT_OpenCards:
		return "open_cards";
	case FeatureTag::FT_CloseCards:
		return "close_cards";
	default:
		PREF_ASSERT(false);
		return "";
	}
}

FeaturesSet CalcFeatures(const GameState& playerView, const GameState& realGame,
		const CardsProbabilities& probArray, Card move, uint32_t ourHero)
{
	static FeaturesRegistry reg;

	FeaturesSet result = reg.CreateEmptySet();
	CardsSet knownCards = playerView.GetKnownCards();
	auto setCardF = [&](FeaturesRange& fr, Card c, float value) {
		result.Set(fr, GetCardBit(c), value);
	};

	for (uint32_t cardBit = 0; cardBit < 32; cardBit++) {
		if (IsCardCovers(CardFromBit(cardBit), playerView.OnDesk(playerView.PlayerWithGreaterCard()), playerView.GetTrump())) {
			setCardF(reg.IsGreaterCard, CardFromBit(cardBit), 1.0f);
		}
	}

	for (uint32_t i = 0, player = ourHero; i < 3; player = (player + 1) % 3, i++) {
		for (auto card : playerView.Hand(player)) {
			setCardF(reg.PlayerCards[i], card, 1.0f);
		}
	}

	for (uint32_t cardBit = 0; cardBit < 32; cardBit++) {
		auto card = CardFromBit(cardBit);
		if (!knownCards.IsInSet(card) || playerView.Out().IsInSet(card)) {
			setCardF(reg.NotInGameCards, CardFromBit(cardBit),
					playerView.Out().IsInSet(card) ? 1.0f : probArray[3][cardBit]);
		}
	}

	for (uint32_t i = 0, player = ourHero; i < 3; player = (player + 1) % 3, i++) {
		if (playerView.OnDesk(player) != NoCard) {
			setCardF(reg.CardsOnDesk[i], playerView.OnDesk(player), 1.0f);
		}
	}

	return result;
}

