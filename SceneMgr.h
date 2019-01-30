#pragma once
#include "Object.h"
#include "Player.h"
#include "Renderer.h"
#include <windows.h>

class SceneMgr
{
	bool invincibility			=0;
	DWORD g_prevTime			=0;
	DWORD currTime				=0;
	DWORD elapsedTime			=0;
	DWORD collisionTime			=0;
public:
	int mynum = 0;
	float alpha = 1.0f;
	bool isPlaying;
	clock_t realtime;
	clock_t colltime[3];
	float Playtime;   // 서버로부터 받아온 시간을 저장 한다. 
	int Obj_num = 0;
	int BulletTrick = 0;
	Renderer *g_Renderer;
	GLuint Player_Texture = NULL;
	GLuint Object_Texture = NULL;
	GLuint UI_Texture = NULL;
	GLuint Life_Texture = NULL;
	GLuint Arrow_Texture = NULL;

	Player p[3];
	Object obj[MAX];
	void InitRender();
	void Collision();
	void Update();
	//void Update(bool up, bool down, bool left, bool right);
	void Render();
	SceneMgr();
	~SceneMgr();
};
