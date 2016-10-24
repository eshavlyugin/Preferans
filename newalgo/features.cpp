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
		: PlayerCards({ FeaturesRange(this, NumCardFeatures, FT_CloseCards)})
		, NotInGameCards(this, NumCardFeatures, FT_CloseCards)
		, CardsOnDesk({ FeaturesRange(this, NumCardFeatures, FT_CommonCards), FeaturesRange(this, NumCardFeatures, FT_CommonCards) })
		, Move(this, NumCardFeatures, FT_Move)
		, IsGreaterCard(this, 32, FT_CommonCards)
		, IsValidMove(this, 32, FT_CommonCards)
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

FeaturesSet CalcFeatures(const GameState& playerView, const GameState& realGame,
		const CardsProbabilities& probArray, Card move, uint32_t ourHero)
{
	static FeaturesRegistry reg;

	FeaturesSet result = reg.CreateEmptySet();
	CardsSet knownCards = playerView.GetKnownCards();
	auto setCardF = [&](FeaturesRange& fr, Card c, float value) {
		result.Set(fr, GetCardBit(c), value);
	};

	for (Suit s = Spades; s != NoSuit; s = (Suit)(uint32_t(s) + 1)) {
		uint32_t realRank = 0;
		for (Rank r = 0; r < 8; ++r) {
			auto card = MakeCard(s, r);
			/*if (playerView.Out().IsInSet(card)) {
				continue;
			}*/
			auto cardForFeature = card; //MakeCard(s, realRank);
			if (IsCardCovers(card, playerView.OnDesk(playerView.PlayerWithGreaterCard()), playerView.GetTrump())) {
				setCardF(reg.IsGreaterCard, cardForFeature, 1.0f);
			}
			setCardF(reg.IsValidMove, cardForFeature, playerView.IsValidMove(card) ? 1.0f : 0.0f);
			bool onDesk = false;
			for (uint32_t i = 0, player = playerView.GetFirstPlayer(); i < 2; player = (player + 1) % 3, i++) {
				if (playerView.OnDesk(player) == card) {
					onDesk = true;
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
		/*for (Rank r = realRank; r < 8; ++r) {
			setCardF(reg.NotInGameCards, MakeCard(s, r), 1.0f);
		}*/
	}

	return result;
}

