#pragma once
class Object
{
public:
	float X, Y;
	float Vx = 0.0f, Vy = 0.0f;
	Object();
	Object(float x,float y,float vx,float vy);
	~Object();
};

