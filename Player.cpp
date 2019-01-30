
#include "stdafx.h"
#include "Player.h"
Player::Player(){}
Player::Player(int n,float x, float y, float r, float g, float b)
{
	num = n;
	X = x;
	Y = y;
	R = r;
	G = g;
	B = b;
	Ranking = 3;
	Score = 0.f;
}


Player::~Player()
{
}
