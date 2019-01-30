/*
Copyright 2017 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/

///////////////////////////////////////
#include "stdafx.h"
#include "SceneMgr.h"
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"
// 소켓 함수 오류 출력 후 종료

//통신 스레드 함수 함수
DWORD WINAPI ConnectThread(LPVOID arg);

// 변수
SceneMgr *scene = new SceneMgr();
KeyState key;
// 초기화 구조체
InitPacket initPacketClient;
// 스레드 핸들값
HANDLE hThread;
// 매 프레임 송수신 구조체
RecvFromServer ReceiveFromeServerPacket;
//서버 아이피 초기화
char serverIp[30] = { "127.0.0.1" };
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// 소켓 함수 오류 출력
void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}
//넘어온 초기화패킷 값을 필요한 곳에 복사, 저장한다.
void copyinit()
{
	//실시간 저장
	scene->mynum = initPacketClient.player_num;
	scene->realtime = initPacketClient.time;
	//각 플레이어 정보 저장
	for (int i = 0; i < 3; ++i)
	{
		scene->p[i].X = initPacketClient.player[i].X;
		scene->p[i].Y = initPacketClient.player[i].Y;
		scene->p[i].R = initPacketClient.player[i].R;
		scene->p[i].G = initPacketClient.player[i].G;
		scene->p[i].B = initPacketClient.player[i].B;
		scene->p[i].life = initPacketClient.player[i].life;
	}
	//오브젝트 정보 저장
	for (int i = 0; i < MAX; ++i)
	{
		scene->obj[i].X = initPacketClient.object[i].X;
		scene->obj[i].Y = initPacketClient.object[i].Y;
		scene->obj[i].Vx = initPacketClient.object[i].Vx;
		scene->obj[i].Vy = initPacketClient.object[i].Vy;
	}
}

//매 프레임마다 넘어오는 패킷의 정보를 복사, 저장한다.
void UpdateClientfromServerPacket()
{
	//게임중인 상태 불변수를 저장
	scene->isPlaying = ReceiveFromeServerPacket.isPlaying;
	//매프레임 플레이어 정보 저장
	for (int i = 0; i < 3; ++i)
	{
		scene->p[i].X = ReceiveFromeServerPacket.player[i].X;
		scene->p[i].Y = ReceiveFromeServerPacket.player[i].Y;
		scene->p[i].R = ReceiveFromeServerPacket.player[i].R;
		scene->p[i].G = ReceiveFromeServerPacket.player[i].G;
		scene->p[i].B = ReceiveFromeServerPacket.player[i].B;
		scene->p[i].life = ReceiveFromeServerPacket.player[i].life;
		scene->p[i].IsCollision = ReceiveFromeServerPacket.player[i].IsCollision;
	
		scene->p[i].Ranking = ReceiveFromeServerPacket.player[i].Ranking;
		scene->p[i].Score = ReceiveFromeServerPacket.player[i].Score;
		scene->p[i].isGameEnd = ReceiveFromeServerPacket.player[i].isGameEnd;

		scene->p[i].num = ReceiveFromeServerPacket.player[i].num;
	}
	//매 프레임 오브젝트 정보 저장
	for (int i = 0; i < MAX; ++i)
	{
		scene->obj[i].X = ReceiveFromeServerPacket.object[i].X;
		scene->obj[i].Y = ReceiveFromeServerPacket.object[i].Y;
		scene->obj[i].Vx = ReceiveFromeServerPacket.object[i].Vx;
		scene->obj[i].Vy = ReceiveFromeServerPacket.object[i].Vy;
	}

	//플레이어 무적처리에 대한 검사
	for (int i = 0; i < 3; ++i)
	{
		if (scene->p[i].IsCollision)
		{
			scene->p[i].invincibility = true;
			scene->colltime[i] = scene->realtime + 3000;
		}
	}
}

//키 입력 함수
void keyInput()
{
	key.up = false;
	key.down = false;
	key.right = false;
	key.left = false;
	if (GetAsyncKeyState(VK_UP)) { key.up = true; }
	if (GetAsyncKeyState(VK_DOWN)) {key.down = true; }
	if (GetAsyncKeyState(VK_LEFT)) { key.left = true; }
	if (GetAsyncKeyState(VK_RIGHT)) {key.right = true; }
	if (GetAsyncKeyState(0x52)) { key.Restart = true; }
}

//렌더링을 위한 함수
void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	keyInput();
	scene->Render();
	glutSwapBuffers();
}

// 프레임 조절을 위한 함수
void RenderTime(int t)
{
	glutTimerFunc(TIME, RenderTime, 1);
	RenderScene();
}

//----- 
// main
//-----
int main(int argc, char **argv)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		exit(1);

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// 서버에게 접속하기 위한 IP 주소 입력
	cout << "IP 주소를 입력 해 주세요. : ";
	cin >> serverIp;

	// 스레드 생성
	hThread = CreateThread(NULL, 0, ConnectThread, (LPVOID)sock, 0, NULL);
	if (hThread == NULL)
	{
		closesocket(sock);
	}
	else
	{
		CloseHandle(hThread);
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(400, 100);//optional
	glutInitWindowSize(WSIZE, WSIZE); //optional
	glutCreateWindow("Network Game Programming KPU");

	glewInit();

	// 씬 초기화
	scene->InitRender();

	glutDisplayFunc(RenderScene);
	glutTimerFunc(TIME, RenderTime, 1);
	glutMainLoop();
	
	return 0;
}

// 서버와 통신하기 위한 클라이언트 스레드
DWORD WINAPI ConnectThread(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	//클라이언트 통신
	int retval;

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(serverIp);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	int initPacketSize = 0;
	bool isNeedInit = true;


	int i = 0;

	while (1)
	{
		//초기화가 되었다면 수행.
		if (isNeedInit == true)
		{
			//-------------------------------------
			// 서버와 데이터 통신
			// send() & recv 를 사용할 부분

			// InitPacket 사이즈 수신
			retval = recv(sock, (char*)&initPacketSize, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				exit(1);
			}
			ZeroMemory(buf, BUFSIZE);
		
			// InitPacket 정보 수신
			retval = recv(sock, buf, initPacketSize, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				exit(1);
			}
			// initPacketClient 패킷 구조체에 서버에서 넘어온 InitPacket 정보를 복사.
			memcpy((void*)&initPacketClient, buf, BUFSIZE);

			// initPacketClient 구조체에 있는 값을 실제 필요한 구조체에 복사하는 함수
			copyinit();
			
			//초기화 변수 false.
			isNeedInit = false;
		}
		// 초기화할 필요가 없다면 매프레임마다 데이터 통신 수행한다. 
		else
		{
			ZeroMemory(buf, BUFSIZE);
			// 키 버퍼를 복사
			memcpy(buf, (void*)&key, sizeof(key));

			// 키 입력 정보를 서버에게 전송
			retval = send(sock, buf, sizeof(key), 0);
			
			// 키 입력에 따라, 혹은 업데이트 된 정보들에 대한 패킷 수신
			retval = recv(sock, buf, sizeof(ReceiveFromeServerPacket), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				exit(1);
			}
			// 업데이트 된 정보들을 가지고 있는 버퍼를 수신전용 패킷구조체에 복사.
			memcpy((void*)&ReceiveFromeServerPacket, buf, BUFSIZE);
			
			// 패킷에 대한 정보를 복사해주기 위한 함수
			UpdateClientfromServerPacket();
			
			//실시간 검색을 위한 함수.
			scene->realtime = clock();
			//실제 플레이 시간을 복사
			scene->Playtime = ReceiveFromeServerPacket.time;

		}
	}

	//종료 검사
	bool end = false;
	if (end)
	{
		closesocket(sock);
		//윈속 종료
		WSACleanup();
	}
}


