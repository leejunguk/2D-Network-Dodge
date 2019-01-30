
#include "stdafx.h"
#include "SceneMgr.h"


SceneMgr::SceneMgr() {}
SceneMgr::~SceneMgr() { delete g_Renderer; }
void SceneMgr::Collision()
{

}
void SceneMgr::Update()
{
}

void SceneMgr::InitRender()
{
	g_Renderer = new Renderer(WSIZE, WSIZE);
	Player_Texture = g_Renderer->CreatePngTexture("Textures/Plane.png");
	Object_Texture = g_Renderer->CreatePngTexture("Textures/bullet.png");
	UI_Texture = g_Renderer->CreatePngTexture("Textures/UI.png");
	Life_Texture = g_Renderer->CreatePngTexture("Textures/life.png");
	Arrow_Texture = g_Renderer->CreatePngTexture("Textures/this.png");

};
// 서버에서 구현
void SceneMgr::Render()
{
	// 게임 대기중이면 플레이어 접속 기다림 문자 출력
	if (isPlaying == false)
	{
		g_Renderer->DrawText(-70, 50, GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, "Connecting Player...");
	}

	// 문자열 출력을 위한 변수들
	int num = 50;
	char buffer[65];
	int DummyClock = 0;
	char integerTimebuffer[65];
	char floatTimebuffer[65];
	int fbuffer;

	// 총알 속임수 변수 10초마다 1 증가
	BulletTrick = int(Playtime) / 10000;
	itoa(num + BulletTrick, buffer, 10);

	// 시간을 실수 부분 , 정수 부분 계산
	DummyClock = DummyClock + int(Playtime);
	int part_integer = DummyClock * 0.001;
	int part_float = DummyClock % 1000;
	itoa(part_integer, integerTimebuffer, 10);
	itoa(part_float, floatTimebuffer, 10);

	// 총알 수 출력
	g_Renderer->DrawText(-70, (WSIZE * 0.5) - (UISIZE * 0.5) , GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, "bullet : ");
	g_Renderer->DrawText( 0 , (WSIZE * 0.5) - (UISIZE * 0.5), GLUT_BITMAP_TIMES_ROMAN_24, 1 , 1, 1, buffer);

	// 시간 출력
	g_Renderer->DrawText(100, 330, GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, "time : ");
	g_Renderer->DrawText(200, 330, GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, integerTimebuffer);
	g_Renderer->DrawText(225, 330, GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, ".");
	g_Renderer->DrawText(230, 330, GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, floatTimebuffer);

	//화살표 그리기
	if(p[mynum].life > 0)
		g_Renderer->DrawTexturedRect(p[mynum].X, p[mynum].Y + PLAYER_SIZE * 0.5 + 10, 0, 10, 1, 1, 1, 1.0f, Arrow_Texture, 0.01f);

	
	// 게임 종료시 출력 정보
	if (p[0].isGameEnd == true && p[1].isGameEnd == true && p[2].isGameEnd == true)
	{
		for (int i = 0; i < 3; ++i)
		{
			char RankingBuf[56];
			itoa(p[i].Ranking, RankingBuf, 10);
			char RankingNum[5];
			itoa(i, RankingNum, 10);
			char OutputScore[40];
			itoa(p[i].Score, OutputScore, 10);
			g_Renderer->DrawText(-300, 100 * (-i + 1), GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, "Player : ");
			g_Renderer->DrawText(-200, 100 * (-i + 1), GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, RankingNum);
			g_Renderer->DrawText(-100, 100 * (-i + 1), GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, "Ranking : ");
			g_Renderer->DrawText(0, 100 * (-i + 1), GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, RankingBuf);
			g_Renderer->DrawText(70, 100 * (-i + 1), GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, "Score : ");
			g_Renderer->DrawText(150,100 * (-i + 1), GLUT_BITMAP_TIMES_ROMAN_24, 1, 1, 1, OutputScore);
			g_Renderer->DrawTexturedRect(250, 100 * (-i + 1) + 10, 0, PLAYER_SIZE,p[i].R, p[i].G, p[i].B, 1.0f, Player_Texture, 0.01f);
		}
	}

	// 실시간 플레이어와 오브젝트 출력
	for (int i = 0; i < 3; ++i)
	{
		if (p[i].life > 0)
		{
			if (realtime <= colltime[i])
			{
				if (alpha)
					alpha = 0.5f;
				else if(alpha<1)
					alpha = 1.0f;
				g_Renderer->DrawTexturedRect(p[i].X, p[i].Y, 0.0f, PLAYER_SIZE, p[i].R, p[i].G, p[i].B,alpha, Player_Texture, 0.0f);
			}
			else
			{
				g_Renderer->DrawTexturedRect(p[i].X, p[i].Y, 0.0f, PLAYER_SIZE, p[i].R, p[i].G, p[i].B, 1.0f, Player_Texture, 0.0f);
				p[i].invincibility = 0;
				}
		}
	}
	for (int i = 0; i < MAX; ++i)
	{
		g_Renderer->DrawTexturedRect(obj[i].X, obj[i].Y, 0.0f, BULLET_SIZE, 1.0f, 1.0f, 1.0f, 1.0f, Object_Texture, 0.1f);
	}

	// UI 출력
	for (int i = -WSIZE / 2; i < WSIZE / 2; ++i)
		g_Renderer->DrawSolidRect(i, -WSIZE / 2 + UISIZE, 0.0f, 1, 0.0f, 1.0f, 1.0f, 1.0f, 0.9f);
	for (int i = -WSIZE / 2; i < WSIZE / 2; ++i)
		g_Renderer->DrawSolidRect(i, WSIZE / 2 - UISIZE, 0.0f, 1, 0.0f, 1.0f, 1.0f, 1.0f, 0.9f);

	g_Renderer->DrawTexturedRect(-125.0f, -WSIZE / 2 + UISIZE / 2, 0.0f, UISIZE, 1.0f, 1.0f, 1.0f, 1.0f, UI_Texture, 0.05f);
	g_Renderer->DrawTexturedRect(-125.0f, -WSIZE / 2 + UISIZE / 2, 0.0f, UISIZE-25, p[0].R, p[0].G, p[0].B, 1.0f, Player_Texture, 0.01f);
	for (int i = 0; i < p[0].life; ++i)
	{
		g_Renderer->DrawTexturedRect(-125.0f+ 35+25*(i), -WSIZE / 2 + UISIZE / 2, 0.0f, 20, 1.0f, 1.0f, 1.0f, 1.0f, Life_Texture, 0.01f);
	}

	g_Renderer->DrawTexturedRect(0.0f, -WSIZE / 2 + UISIZE / 2, 0.0f, UISIZE, 1.0f, 1.0f, 1.0f, 1.0f, UI_Texture, 0.05f);
	g_Renderer->DrawTexturedRect(0.0f, -WSIZE / 2 + UISIZE / 2, 0.0f, 25, p[1].R, p[1].G, p[1].B, 1.0f, Player_Texture, 0.01f);
	for (int i = 0; i < p[1].life; ++i)
	{
		g_Renderer->DrawTexturedRect(-0.0f + 35 +25* (i ), -WSIZE / 2 + UISIZE / 2, 0.0f, 20, 1.0f, 1.0f, 1.0f, 1.0f, Life_Texture, 0.01f);
	}

	g_Renderer->DrawTexturedRect(125.0f, -WSIZE / 2 + UISIZE / 2, 0.0f, UISIZE, 1.0f, 1.0f, 1.0f, 1.0f, UI_Texture, 0.05f);
	g_Renderer->DrawTexturedRect(125.0f, -WSIZE / 2 + UISIZE / 2, 0.0f, 25, p[2].R, p[2].G, p[2].B, 1.0f, Player_Texture, 0.01f);
	for (int i = 0; i < p[2].life; ++i)
	{
		g_Renderer->DrawTexturedRect(125.0f + 35 +25* (i), -WSIZE / 2 + UISIZE / 2, 0.0f, 20, 1.0f, 1.0f, 1.0f, 1.0f, Life_Texture, 0.01f);
	}
}

