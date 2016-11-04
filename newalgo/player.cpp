/*
 * player.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: zhenyok
 */

#include "player.h"

#include "generate.h"
#include "monte_carlo.h"
#include "train_model.h"
#include "utils.h"


class HumanPlayer: public IPlayer {
public:
	void OnNewLayout(const GameState& state) override final {
		state_ = state;
	}
	void OnMove(Card c) override final {
		state_.MakeMove(c);
	}
	void OnNewXRayLayout(const GameState&) override final {
	}
	Card DoMove(float* /*moveValue*/) override final {
		string str;
		bool valid = false;
		Card res;
		while (!valid) {
			cout << "Your move: ";
			cin >> str;
			if (str.size() != 2) {
				cout << "Card always have length 2" << endl;
				continue;
			}
			valid = true;
			Rank rank;
			Suit suit;
			switch (str[0]) {
			case '7':
				rank = 0;
				break;
			case '8':
				rank = 1;
				break;
			case '9':
				rank = 2;
				break;
			case 'T':
				rank = 3;
				break;
			case 'J':
				rank = 4;
				break;
			case 'Q':
				rank = 5;
				break;
			case 'K':
				rank = 6;
				break;
			case 'A':
				rank = 7;
				break;
			default:
				valid = false;
				cout << "Invalid rank " << str[0] << endl;
				continue;
			}

			switch (str[1]) {
			case 's':
				suit = Spades;
				break;
			case 'c':
				suit = Clubs;
				break;
			case 'd':
				suit = Diamonds;
				break;
			case 'h':
				suit = Hearts;
				break;
			default:
				valid = false;
				cout << "Suit " << str[1] << " is not valid" << endl;
				continue;
			}
			if (valid) {
				res = MakeCard(suit, rank);
				if (!state_.IsValidMove(res)) {
					cout << "Invalid move " << CardToString(res) << endl;
					valid = false;
				}
			}
		}
		return res;
	}
	const GameState& GetStateView() const override {
		return state_;
	}

	shared_ptr<IPlayer> Clone() override {
		return shared_ptr<IPlayer>(new HumanPlayer(*this));
	}

private:
	GameState state_;
};

inline void FillKnownCardsArray(const GameState& state,
		array<uint8_t, 32>& knownCards) {
	for (uint32_t bit = 0; bit < 32; bit++) {
		auto card = CardFromBit(bit);
		for (uint32_t player = 0; player < 3; player++) {
			if (state.Hand(player).IsInSet(card)) {
				knownCards[bit] = player + 1;
			}
		}
		if (state.Out().IsInSet(card)) {
			knownCards[bit] = 4;
		}
	}
}

inline void FillRandomProbabilityArray(const GameState& state,
		const array<uint8_t, 32>& knownCards, CardsProbabilities& res) {
	for (uint32_t i = 0; i < 32; i++) {
		if (knownCards[i] != 0) {
			for (uint32_t j = 0; j < 4; j++) {
				res[j][i] = 0.0f;
			}
			res[knownCards[i] - 1][i] = 1.0f;
		} else {
			auto moveNumber = state.GetMoveNumber();
			float sum = 2.0f;
			res[3][i] = 2.0f;
			for (uint32_t player = 0; player < 3; player++) {
				res[player][i] = (state.OnDesk(player) == Card(0) ? 10 : 9)
						- moveNumber;
				sum += res[player][i];
			}
			for (uint32_t j = 0; j < 4; j++) {
				res[j][i] /= sum;
			}
		}
	}
}

class AiPlayer: public IPlayer {
public:
	void SetMovePredictor(shared_ptr<IModel> model) {
		playRandom_ = false;
		movePredictor_ = model;
	}

	void OnNewXRayLayout(const GameState& game) override {
		xrayLayout_.reset(new GameState(game));
	}

	void OnNewLayout(const GameState& state) override final {
		state_ = state;
		firstPlayer_ = state.GetFirstPlayer();
		for (uint32_t i = 0; i < 3; ++i) {
			if (!state_.IsHandClosed(i)) {
				ourHero_ = i;
			}
		}
		fill(knownCards_.begin(), knownCards_.end(), 0);
		FillKnownCardsArray(state_, knownCards_);
	}

	void OnMove(Card card) override final {
		knownCards_[GetCardBit(card)] = 4; // out
		state_.MakeMove(card);
		if (xrayLayout_.get()) {
			xrayLayout_->MakeMove(card);
		}
	}

	Card DoMove(float* /*moveValue*/) override final {
		if (playRandom_) {
			return MakeRandomMove();
		}
		auto valid = state_.GenValidMoves();
		auto weights = movePredictor_->Predict(CalcFeatures(state_, NoCard, ourHero_, FT_Playing));
		Card bestMove = NoCard;
		float best = -1e10f;
		for (auto card : valid) {
			auto weight = weights[GetCardBit(card)];
			if (best < weight) {
				best = weight;
				bestMove = card;
			}
		}
		return bestMove;
	}

	shared_ptr<IPlayer> Clone() override {
		return shared_ptr<IPlayer>(new AiPlayer(*this));
	}

	const GameState& GetStateView() const override {
		return state_;
	}

private:
	Card MakeRandomMove() {
		auto validMoves = state_.GenValidMoves();
		uint32_t move = rand() % validMoves.Size();
		auto it = validMoves.begin();
		for (; move > 0; move--, ++it) {
		}
		return *it;
	}

private:
	GameState state_;
	shared_ptr<GameState> xrayLayout_;
	shared_ptr<IModel> movePredictor_;
	array<uint8_t, 32> knownCards_ = {{0}};
	uint32_t ourHero_ = 0;
	uint32_t firstPlayer_ = 0;

	bool playRandom_ = true;
};

class CompositePlayer : public IPlayer {
public:
	CompositePlayer(const vector<pair<shared_ptr<IPlayer>, float>>& players)
		: playersAndProbs_(players)
	{
		float sum = 0.0f;
		for (const auto& player : playersAndProbs_) {
			sum += player.second;
		}
		for (auto& player : playersAndProbs_) {
			player.second /= sum;
		}
	}

	void OnNewXRayLayout(const GameState& game) override {
		for (const auto& pair : playersAndProbs_) {
			pair.first->OnNewXRayLayout(game);
		}
	}

	void OnNewLayout(const GameState& game) override {
		for (const auto& pair : playersAndProbs_) {
			pair.first->OnNewLayout(game);
		}
	}

	void OnMove(Card card) override {
		for (const auto& pair : playersAndProbs_) {
			pair.first->OnMove(card);
		}
	}

	Card DoMove(float* moveValue) override {
		float prob = 1.0 * rand() / RAND_MAX;
		for (const auto& pair : playersAndProbs_) {
			if (prob + 0.00001f > pair.second) {
				return pair.first->DoMove(moveValue);
			}
			prob -= pair.second;
		}
		PREF_ASSERT(false && "something wrong with the algorithm");
		return NoCard;
	}

	shared_ptr<IPlayer> Clone() override {
		auto playersTmp = playersAndProbs_;
		for (auto& pair : playersTmp) {
			pair.first = pair.first->Clone();
		}
		return std::shared_ptr<IPlayer>(new CompositePlayer(playersTmp));
	}

	// For training model
	const GameState& GetStateView() const override {
		return playersAndProbs_[0].first->GetStateView();
	}

private:
	vector<pair<shared_ptr<IPlayer>, float>> playersAndProbs_;
};

std::shared_ptr<IPlayer> CreatePlayer(const std::string& descr, std::shared_ptr<IModelFactory> modelFactory) {
	if (descr == "human") {
		return std::shared_ptr<IPlayer>(new HumanPlayer());
	} else if (descr == "monte_carlo" || descr == "monte_carlo2") {
		vector<shared_ptr<IPlayer>> players;
		for (uint32_t i = 0; i < 3; i++) {
			players.push_back(CreatePlayer("random", modelFactory));
		}
		return std::shared_ptr<IPlayer>(new MonteCarloPlayer(players, "models/expected_score.tsv", descr == "monte_carlo"));
	} else {
		vector<string> playerParts = utils::split(descr, ',');
		vector<pair<shared_ptr<IPlayer>, float>> players;
		for (const auto& playerPart : playerParts) {
			players.push_back(pair<shared_ptr<IPlayer>, float>());
			auto& pair = players.back();
			vector<string> descrAndProb = utils::split(playerPart, ';');
			if (descrAndProb.size() == 1) {
				pair.second = 1.0f;
			} else if (descrAndProb.size() == 2) {
				pair.second = std::stod(descrAndProb[1]);
			} else {
				PREF_ASSERT(false && "Expected in format descr;weight");
			}

			auto modelName = descrAndProb[0];
			AiPlayer* player(new AiPlayer());
			if (modelName != "random") {
				player->SetMovePredictor(modelFactory->CreateModel(modelName));
			}
			pair.first = shared_ptr<IPlayer>(player);
		}

		if (players.size() == 1) {
			return players[0].first;
		} else {
			return shared_ptr<IPlayer>(new CompositePlayer(players));
		}
	}
}

