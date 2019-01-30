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
// ���� �Լ� ���� ��� �� ����

//��� ������ �Լ� �Լ�
DWORD WINAPI ConnectThread(LPVOID arg);

// ����
SceneMgr *scene = new SceneMgr();
KeyState key;
// �ʱ�ȭ ����ü
InitPacket initPacketClient;
// ������ �ڵ鰪
HANDLE hThread;
// �� ������ �ۼ��� ����ü
RecvFromServer ReceiveFromeServerPacket;
//���� ������ �ʱ�ȭ
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
// ���� �Լ� ���� ���
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
// ����� ���� ������ ���� �Լ�
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
//�Ѿ�� �ʱ�ȭ��Ŷ ���� �ʿ��� ���� ����, �����Ѵ�.
void copyinit()
{
	//�ǽð� ����
	scene->mynum = initPacketClient.player_num;
	scene->realtime = initPacketClient.time;
	//�� �÷��̾� ���� ����
	for (int i = 0; i < 3; ++i)
	{
		scene->p[i].X = initPacketClient.player[i].X;
		scene->p[i].Y = initPacketClient.player[i].Y;
		scene->p[i].R = initPacketClient.player[i].R;
		scene->p[i].G = initPacketClient.player[i].G;
		scene->p[i].B = initPacketClient.player[i].B;
		scene->p[i].life = initPacketClient.player[i].life;
	}
	//������Ʈ ���� ����
	for (int i = 0; i < MAX; ++i)
	{
		scene->obj[i].X = initPacketClient.object[i].X;
		scene->obj[i].Y = initPacketClient.object[i].Y;
		scene->obj[i].Vx = initPacketClient.object[i].Vx;
		scene->obj[i].Vy = initPacketClient.object[i].Vy;
	}
}

//�� �����Ӹ��� �Ѿ���� ��Ŷ�� ������ ����, �����Ѵ�.
void UpdateClientfromServerPacket()
{
	//�������� ���� �Һ����� ����
	scene->isPlaying = ReceiveFromeServerPacket.isPlaying;
	//�������� �÷��̾� ���� ����
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
	//�� ������ ������Ʈ ���� ����
	for (int i = 0; i < MAX; ++i)
	{
		scene->obj[i].X = ReceiveFromeServerPacket.object[i].X;
		scene->obj[i].Y = ReceiveFromeServerPacket.object[i].Y;
		scene->obj[i].Vx = ReceiveFromeServerPacket.object[i].Vx;
		scene->obj[i].Vy = ReceiveFromeServerPacket.object[i].Vy;
	}

	//�÷��̾� ����ó���� ���� �˻�
	for (int i = 0; i < 3; ++i)
	{
		if (scene->p[i].IsCollision)
		{
			scene->p[i].invincibility = true;
			scene->colltime[i] = scene->realtime + 3000;
		}
	}
}

//Ű �Է� �Լ�
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

//�������� ���� �Լ�
void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	keyInput();
	scene->Render();
	glutSwapBuffers();
}

// ������ ������ ���� �Լ�
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
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		exit(1);

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// �������� �����ϱ� ���� IP �ּ� �Է�
	cout << "IP �ּҸ� �Է� �� �ּ���. : ";
	cin >> serverIp;

	// ������ ����
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

	// �� �ʱ�ȭ
	scene->InitRender();

	glutDisplayFunc(RenderScene);
	glutTimerFunc(TIME, RenderTime, 1);
	glutMainLoop();
	
	return 0;
}

// ������ ����ϱ� ���� Ŭ���̾�Ʈ ������
DWORD WINAPI ConnectThread(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	//Ŭ���̾�Ʈ ���
	int retval;

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(serverIp);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE];
	int initPacketSize = 0;
	bool isNeedInit = true;


	int i = 0;

	while (1)
	{
		//�ʱ�ȭ�� �Ǿ��ٸ� ����.
		if (isNeedInit == true)
		{
			//-------------------------------------
			// ������ ������ ���
			// send() & recv �� ����� �κ�

			// InitPacket ������ ����
			retval = recv(sock, (char*)&initPacketSize, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				exit(1);
			}
			ZeroMemory(buf, BUFSIZE);
		
			// InitPacket ���� ����
			retval = recv(sock, buf, initPacketSize, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				exit(1);
			}
			// initPacketClient ��Ŷ ����ü�� �������� �Ѿ�� InitPacket ������ ����.
			memcpy((void*)&initPacketClient, buf, BUFSIZE);

			// initPacketClient ����ü�� �ִ� ���� ���� �ʿ��� ����ü�� �����ϴ� �Լ�
			copyinit();
			
			//�ʱ�ȭ ���� false.
			isNeedInit = false;
		}
		// �ʱ�ȭ�� �ʿ䰡 ���ٸ� �������Ӹ��� ������ ��� �����Ѵ�. 
		else
		{
			ZeroMemory(buf, BUFSIZE);
			// Ű ���۸� ����
			memcpy(buf, (void*)&key, sizeof(key));

			// Ű �Է� ������ �������� ����
			retval = send(sock, buf, sizeof(key), 0);
			
			// Ű �Է¿� ����, Ȥ�� ������Ʈ �� �����鿡 ���� ��Ŷ ����
			retval = recv(sock, buf, sizeof(ReceiveFromeServerPacket), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				exit(1);
			}
			// ������Ʈ �� �������� ������ �ִ� ���۸� �������� ��Ŷ����ü�� ����.
			memcpy((void*)&ReceiveFromeServerPacket, buf, BUFSIZE);
			
			// ��Ŷ�� ���� ������ �������ֱ� ���� �Լ�
			UpdateClientfromServerPacket();
			
			//�ǽð� �˻��� ���� �Լ�.
			scene->realtime = clock();
			//���� �÷��� �ð��� ����
			scene->Playtime = ReceiveFromeServerPacket.time;

		}
	}

	//���� �˻�
	bool end = false;
	if (end)
	{
		closesocket(sock);
		//���� ����
		WSACleanup();
	}
}


