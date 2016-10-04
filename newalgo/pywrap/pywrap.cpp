#include <boost/python.hpp>
#include "../common.h"
#include "../features.h"

CardsSet& (CardsSet::*AddCard)(Card)    = &CardsSet::Add;
CardsSet& (CardsSet::*AddCardsSet)(CardsSet&) = &CardsSet::Add;

namespace bp = boost::python;

GameState* MakeCardsSet(const bp::list& vec, const bp::object& playerRaw, const bp::object& suitRaw) {
	if (bp::len(vec) != 3) {
		throw runtime_error("expected 3 arguments");
//		/assert(false, "expected 3 arguments");
	}
	uint32_t player = bp::extract<uint32_t>(playerRaw)();
	Suit suit = (Suit)bp::extract<uint32_t>(suitRaw)();

	return new GameState({bp::extract<CardsSet>(vec[0]), bp::extract<CardsSet>(vec[1]), bp::extract<CardsSet>(vec[2])}, player, suit);
}

bp::list CalcFeaturesPy(const GameState& gameState, uint32_t ourHero) {
	static CardsProbabilities probs;
	auto features = CalcFeatures(gameState, gameState, probs, NoCard, ourHero);
	bp::list result;
	for (const auto& feature : features.GetFeatures()) {
		result.append(feature.value_);
	}
	return result;
}

BOOST_PYTHON_MODULE(Pref_pywrap)
{
    using namespace boost::python;

    boost::python::class_<CardsSet>("CardsSet", init<>())
    		.def(init<uint32_t>())
    		.def("IsInSet", &CardsSet::IsInSet)
    		.def("Add", AddCard, return_internal_reference<>())
			.def("Add", AddCardsSet, return_internal_reference<>());

    boost::python::class_<GameState>("GameState", no_init)
    		.def("__init__", boost::python::make_constructor(MakeCardsSet))
			.def("Hand", &GameState::Hand)
    		.def("MakeMove", &GameState::MakeMove)
    		.def("IsValidMove", &GameState::IsValidMove);

    def("CardToString", CardToString);
    def("StringToCard", StringToCard);
    def("CalcFeatures", CalcFeaturesPy);
}
