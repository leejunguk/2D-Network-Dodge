#pragma once
#include "Defines.h"
#include "Object.h"
#include "Player.h"


struct KeyState
{
	bool up;
	bool left;
	bool down;
	bool right;
	bool Restart;
};

struct InitPacket
{
	int player_num;
	Player player[PLAYERNUM];
	Object object[MAX];
	float time;
};

struct SendToClient
{
	bool isPlaying;
	Player player[PLAYERNUM];
	Object object[MAX];
	float time;
};

