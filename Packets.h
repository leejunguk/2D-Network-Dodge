#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Player.h"


struct End
{
	Player player;
	float time;
	bool end;
};

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
	Player player[3];
	Object object[200];
	bool start;
	float time;
};


struct RecvFromServer
{
	bool isPlaying = false;
	Player player[3];
	Object object[200];
	float time;
	int objectNum;
};