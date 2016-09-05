#include "generate.h"

GameState GenLayout() {
	std::array<CardsSet, 3> hands;
	std::vector<bool> used(32);

	uint32_t remains = 32;
	for (uint32_t i = 0; i < 3; i++) {
		for (uint32_t j = 0; j < 10; j++) {
			uint32_t rnd = rand() % remains;
			uint32_t cardBit = 0;
			for (; rnd > 0 || used[cardBit]; cardBit++) {
				rnd -= (used[cardBit] ? 0 : 1);
			}
			used[cardBit] = true;
			hands[i].Add(CardFromBit(cardBit));
			remains--;
		}
	}
	return GameState(hands, rand() % 3, NoSuit);
}
