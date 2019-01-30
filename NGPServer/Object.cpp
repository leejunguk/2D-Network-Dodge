
#include "Defines.h"
#include "Object.h"

Object::Object(){}
Object::Object(float x, float y, float vx, float vy)
{
	X = x;
	Y = y;
	Vx = vx;
	Vy = vy;
}


Object::~Object()
{
}
