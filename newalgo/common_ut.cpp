#include "common.h"

#include <gtest/gtest.h>

TEST(CommonUT, CardsSetTest) {
	CardsSet s;
	auto card1 = MakeCard(Hearts, 3); // 9h
	auto card2 = MakeCard(Spades, 4); // Ts 
	auto card3 = MakeCard(Clubs, 7); // Ac
	s.Add(card1);
	s.Add(card2);
	ASSERT_TRUE(s.IsInSet(card1));
	ASSERT_TRUE(s.IsInSet(card2));
	ASSERT_FALSE(s.IsInSet(card3));
}
