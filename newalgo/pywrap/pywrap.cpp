#include <boost/python/numeric.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include "../common.h"
#include "../features.h"
#include "../gamemgr.h"
#include "../generate.h"
#include "../player.h"
#include "../train_model.h"

CardsSet& (CardsSet::*AddCardsSet)(CardsSet) = &CardsSet::Add;

namespace bp = boost::python;

void PyAssertHandler(const char* assert, const char* file, long line) {
	ostringstream ost;
	ost << "Assertion in preflib: \"" << assert << "\" failed. " << file << ":" << line;
	throw runtime_error(ost.str());
}

class PyPlayer : public IPlayer, public bp::wrapper<IPlayer> {
public:
	void OnNewLayout(const GameState& game) override {
		this->get_override("on_new_layout")(game);
	}
	void OnNewXRayLayout(const GameState& game) override {
		this->get_override("on_new_xray_layout")(game);
	}
	void OnMove(Card card) override {
		this->get_override("on_move")(card);
	}
	Card DoMove(float* /*moveValue*/) override {
		bp::object res = this->get_override("do_move")();
		return StringToCard(bp::extract<string>(res));
	}
	shared_ptr<IPlayer> Clone() override {
		return shared_ptr<IPlayer>(this);
	}
	const GameState& GetStateView() const override {
		PREF_ASSERT(false && "data gathering supported only in c++ not in the python code");
		static GameState dummy;
		return dummy;
	}
	void GetCardProbabilities(CardsProbabilities&) override {
		PREF_ASSERT(false && "data gathering is supported only in c++ not in the python code");
	}
};

CardsSet* MakeCardsSet(bp::object& o) {
	CardsSet* result = new CardsSet();
    bp::stl_input_iterator<string> begin(o), end;
    for (auto it = begin; it != end; it++) {
    	result->Add(StringToCard(*it));
    }
    return result;
}

GameState* MakeGameState(const bp::list& vec, uint32_t player, const std::string& suit) {
	PREF_ASSERT(bp::len(vec) == 3 && suit.size() == 1 && "wrong arguments list");
	return new GameState({bp::extract<CardsSet>(vec[0]), bp::extract<CardsSet>(vec[1]), bp::extract<CardsSet>(vec[2])}, player, CharToSuit(suit[0]));
}

bp::list CalcFeaturesPy(const GameState& gameState, uint32_t ourHero, const Card& card, const std::string& featureType) {
	static CardsProbabilities probs;
	auto features = CalcFeatures(gameState, probs, card, ourHero, StringToTag(featureType));
	bp::list result;
	for (float feature : features.GetFeatures()) {
		result.append(feature);
	}
	return result;
}

bp::list GetScoresWrap(const GameState& gameState) {
	bp::list result;
	for (uint32_t score : gameState.GetScores()) {
		result.append(score);
	}
	return result;
}

bp::list CardsSetToList(const CardsSet& cs) {
	bp::list result;
	for (Card move : cs) {
		result.append(CardToString(move));
	}
	return result;
}

bp::list GenValidMovesWrap(const GameState& gameState) {
	return CardsSetToList(gameState.GenValidMoves());
}

template<class T>
T CopyObject(const T& t) {
	return t;
}

uint32_t GetCardBitWrap(const std::string& card) {
	return GetCardBit(StringToCard(card));
}

void CloseHandsWrap(GameState& gs, bp::list list) {
	gs.CloseHands(vector<uint8_t>(bp::stl_input_iterator<uint8_t>(list), bp::stl_input_iterator<uint8_t>()));
}

GameManager* CreateGameManager(bp::list l) {
	return new GameManager(vector<shared_ptr<IPlayer>>(bp::stl_input_iterator<shared_ptr<IPlayer>>(l), bp::stl_input_iterator<shared_ptr<IPlayer>>()));
}

BOOST_PYTHON_MODULE(Pref_pywrap)
{
    using namespace boost::python;

    SetAssertHandler(&PyAssertHandler);

    srand(time(nullptr));

    bp::implicitly_convertible<shared_ptr<PyPlayer>, shared_ptr<IPlayer>>();
    bp::implicitly_convertible<Card, std::string>();
    bp::implicitly_convertible<std::string, Card>();
    bp::implicitly_convertible<Card, CardsSet>();

    boost::python::class_<Card>("Card", no_init)
    		.def("__str__", &CardToString);

    boost::python::class_<CardsSet>("CardsSet", no_init)
    		.def("__init__", bp::make_constructor(MakeCardsSet))
    		.def("__contains__", &CardsSet::IsInSet)
			.def("__iter__", bp::iterator<CardsSet>())
			.def("Add", AddCardsSet, return_internal_reference<>())
			.def("ToArray", CardsSetToList);

    boost::python::class_<GameState>("GameState", no_init)
    		.def("__init__", bp::make_constructor(MakeGameState))
			.def("__copy__", &CopyObject<GameState>)
			.def("Hand", &GameState::Hand)
    		.def("MakeMove", &GameState::MakeMove)
    		.def("IsValidMove", &GameState::IsValidMove)
			.def("GenValidMoves", &GenValidMovesWrap)
    		.def("GetScores", GetScoresWrap)
			.def("Hand", &GameState::Hand)
			.def("OnDesk", &GameState::OnDesk)
			.def("GetMoveNumber", &GameState::GetMoveNumber)
			.def("GetCurPlayer", &GameState::GetCurPlayer)
			.def("IsHandClosed", &GameState::IsHandClosed)
			.def("CloseHands", &CloseHandsWrap);

    boost::python::class_<IPlayer, shared_ptr<IPlayer>, boost::noncopyable>("Player", no_init)
    		.def("create", CreatePlayer)
			.staticmethod("create");

    boost::python::class_<PyPlayer, shared_ptr<PyPlayer>, boost::noncopyable>("PyPlayer", init<>());

    boost::python::class_<GameManager>("GameManager", no_init)
    		.def("__init__", boost::python::make_constructor(CreateGameManager))
			.def("SetNewLayout", &GameManager::SetNewLayout)
			.def("PlayToTheEnd", &GameManager::PlayToTheEnd)
			.def("PlayForNMoves", &GameManager::PlayForNMoves)
    		.def("GetState", &GameManager::GetState, return_value_policy<bp::copy_const_reference>());

    def("CalcFeatures", CalcFeaturesPy);
    def("GetCardIndex", GetCardBitWrap);
    def("GenLayout", GenLayout);
    def("EncodeMoveIndex", EncodeMoveIndex);
}
