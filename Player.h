#pragma once
class Player
{
public:
	int num;
	float X, Y;
	float R, G, B;
	int life = 3;
	bool IsCollision = false;
	bool invincibility = false;
	int Ranking;
	int Score;
	bool isGameEnd = false;
	Player();
	Player(int n,float x, float y, float r, float g, float b);

	~Player();
};

