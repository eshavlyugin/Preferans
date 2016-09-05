#pragma once

#include "common.h"
#include "gamemgr.h"
#include "player.h"

#include <bits/stdc++.h>

class PlayoutManager {
public:
	PlayoutManager() {
	}

	map<Card, array<float, 3>> Play(const GameManager& gameMgr,
			uint32_t trialsPerMove);
};
