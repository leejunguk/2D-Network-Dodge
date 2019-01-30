#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "Object.h"
#include "Player.h"
#include "Packets.h"
#include <time.h>
using namespace std;

// ����
int player_cnt = -1;
int RankingNumbering = PLAYERNUM;
KeyState key[PLAYERNUM];
CRITICAL_SECTION cs;
Player p[PLAYERNUM];
Object obj[MAX];
InitPacket initPacket;
SendToClient SendToClientPacket;
bool isGameEnd;
bool startcheck = false;

// �̺�Ʈ �ڵ�
HANDLE UpdateEvent[PLAYERNUM];

//�ð� ����
clock_t realtime[PLAYERNUM] = { clock(),clock(),clock()};
clock_t invincibleTime[PLAYERNUM];
clock_t totalPlayTime;
clock_t PlayStartTime;
// �Լ�
void Update(int n);
void ObjectUpdate();
void InitPacketServer();
void UpdateTime();



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
// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ConnectThread(LPVOID arg)
{


	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE];


	// �÷��̾� ī��Ʈ�� 3 �Ѿ�� ������ �Լ��� ������
	if(player_cnt > PLAYERNUM -1 )
		return 0;
	int myThreadPlayerNum = player_cnt;
	initPacket.player_num = player_cnt;
	


	//������ ��ſ� ����� ����
	int packetSize = 0;
	int sendCount = 0;
	int sendCountMod = 0;
	int InitPacketSize = sizeof(InitPacket);
	bool isNeedInit = false;
	
	clock_t elapsetime;
	clock_t gameStartTime;
	clock_t gameLoopTime;
	clock_t GameTime;

	//�÷��̾ ������ ���ö����� ����Ѵ�.

		gameStartTime = clock();
		while (1)
		{
			gameLoopTime = clock();
			//������ �����ֱ����� �ð��� 
			GameTime = gameLoopTime - gameStartTime;
			if ((float)GameTime > WaitTime)
			{
				elapsetime = clock();
				addrlen = sizeof(clientaddr);
				getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

				if (isNeedInit == false)
				{
					// �ʱ�ȭ ��Ŷ ������ ���� ( �������� ) 
					retval = send(client_sock, (char*)&InitPacketSize, sizeof(InitPacketSize), 0);

					ZeroMemory(buf, BUFSIZE);
					memcpy(buf, (void*)&initPacket, InitPacketSize);

					// �ʱ�ȭ ��Ŷ ����
					retval = send(client_sock, buf, InitPacketSize, 0);
					isNeedInit = true;
					SendToClientPacket.isPlaying = true;
				}
				// 30�����ӿ� ���缭 �������� ������ �ۼ���
				else if (elapsetime - realtime[myThreadPlayerNum] > (FRAME)&& player_cnt>=PLAYERNUM-1) //1000dl 1�� 
				{
					realtime[myThreadPlayerNum] = elapsetime;
					ZeroMemory(buf, BUFSIZE);
					// Ű �Է� ��Ŷ ����
					retval = recv(client_sock, buf, sizeof(key[myThreadPlayerNum]), 0);
					// Ű �Է� ��Ŷ ���� ����
					memcpy((void*)&key[myThreadPlayerNum], buf, sizeof(key[myThreadPlayerNum]));
					
					//����� �ڵ�
					if (key[0].Restart == true && key[1].Restart == true && key[2].Restart == true && isGameEnd == true)
					{
						for (int i = 0; i < 3; ++i)
						{
							p[i].X = 0;
							p[i].Y = 0;
							p[i].life = 3;
							p[i].isDead = false;
							p[i].isGameEnd = false;
							p[i].Score = 0;
							p[i].Ranking = 0;
							SendToClientPacket.time = 0;
							startcheck = false;
						}
						isGameEnd = false;
					}

					//�÷��̾ 1���̶��
					if (myThreadPlayerNum == 0)
					{
						//�÷��̾� 3���� �̺�Ʈ ���
						retval = WaitForSingleObject(UpdateEvent[2], INFINITE);
						Update(myThreadPlayerNum);
						ObjectUpdate();
						//�÷��̾� 1���� �̺�Ʈ �Ϸ�
						SetEvent(UpdateEvent[0]);
					}
					//�÷��̾ 2�� �̶��
					if (myThreadPlayerNum == 1)
					{
						//�÷��̾� 1���� �̺�Ʈ ���
						retval = WaitForSingleObject(UpdateEvent[0], INFINITE);
						Update(myThreadPlayerNum);
						ObjectUpdate();
						//�÷��̾� 2���� �̺�Ʈ �Ϸ�
						SetEvent(UpdateEvent[1]);
					}
					// �÷��̾ 3�� �̶��
					if (myThreadPlayerNum == 2)
					{
						//�÷��̾� 2���� �̺�Ʈ ���
						retval = WaitForSingleObject(UpdateEvent[1], INFINITE);
						Update(myThreadPlayerNum);
						ObjectUpdate();
						//�÷��̾� 3���� �̺�Ʈ �Ϸ�
						SetEvent(UpdateEvent[2]);
					}
						ZeroMemory(buf, BUFSIZE);
						//�ð� ������Ʈ
						UpdateTime();

						memcpy(buf, (void*)&SendToClientPacket, sizeof(SendToClientPacket));
						// �� ������ ��Ŷ ���� �۽�
						retval = send(client_sock, buf, sizeof(SendToClientPacket), 0);
				}
			}
			
		}
	return 0;
}
// ������Ʈ Ÿ��
void UpdateTime()
{
	if (!startcheck)
	{
		PlayStartTime = clock();
		startcheck = true;
	}
	clock_t totalPlayTimelaps = clock();

	clock_t PlayTime = totalPlayTimelaps - totalPlayTime;  // �÷��̽ð� 
	SendToClientPacket.time = (float)(PlayTime- PlayStartTime) - WaitTime;     //��Ŷ�� �÷��� �ð� �Ѱ���.
												   //cout << "�ð� ������������" << SendToClientPacket.time << endl;
}

// �÷��̾� Ű �Է� ������Ʈ
void Update(int n)
{
	p[n].num = n;
		if (key[n].up)
		{
			if ((WSIZE - PLAYER_SIZE) / 2 - UISIZE> p[n].Y)
				p[n].Y += PLAYER_SPEED;
		}
		if (key[n].down)
		{
			if (-(WSIZE - PLAYER_SIZE) / 2 + UISIZE < p[n].Y)
				p[n].Y += -PLAYER_SPEED;
		}
		if (key[n].left)
		{
			if (-(WSIZE - PLAYER_SIZE) / 2 < p[n].X)
				p[n].X += -PLAYER_SPEED;
		}
		if (key[n].right)
		{
			if ((WSIZE - PLAYER_SIZE) / 2 > p[n].X)
				p[n].X += PLAYER_SPEED;
		}
}
// ������Ʈ ������Ʈ
void ObjectUpdate()
{
	for (int i = 0; i < MAX; ++i)
	{
		obj[i].X += obj[i].Vx;
		obj[i].Y += obj[i].Vy;

		if (obj[i].X > BOUNDING_ZONE / 2)
			obj[i].Vx = -obj[i].Vx;
		if (obj[i].X < -BOUNDING_ZONE / 2)
			obj[i].Vx = -obj[i].Vx;
		if (obj[i].Y > BOUNDING_ZONE / 2 - UISIZE)
			obj[i].Vy = -obj[i].Vy;
		if (obj[i].Y < -BOUNDING_ZONE / 2 + UISIZE)
			obj[i].Vy = -obj[i].Vy;
		for (int j = 0; j < PLAYERNUM; ++j)
		{
			if (p[j].life <= 0)
				continue;
			if (p[j].invincibility == 1)
			{
				if (invincibleTime[j] + 3000 <= realtime[j])
				{
					p[j].invincibility = 0;
					p[j].IsCollision = 0;
				}
				else
					continue;
			}
			if( (obj[i].X > p[j].X - PLAYER_SIZE / 2) && (obj[i].X < p[j].X + PLAYER_SIZE/2) )
				if ((obj[i].Y > p[j].Y - PLAYER_SIZE / 2) && (obj[i].Y < p[j].Y + PLAYER_SIZE / 2))
				{
					switch (POSIT_RANDOM)
					{
					case 1:
						obj[i].X = CREATE_ZONE / 2 + rand() % 50;
						obj[i].Y = 0 + rand() % 50;
						break;
					case 2:
						obj[i].X = CREATE_ZONE / 2 + rand() % 50;
						obj[i].Y = CREATE_ZONE / 2 + rand() % 50;
						break;
					case 3:
						obj[i].X = 0 + rand() % 50;
						obj[i].Y = CREATE_ZONE / 2 + rand() % 50;
						break;
					case 4:
						obj[i].X = -CREATE_ZONE / 2 + rand() % 50;
						obj[i].Y = CREATE_ZONE / 2 + rand() % 50;
						break;
					case 5:
						obj[i].X = -CREATE_ZONE / 2 + rand() % 50;
						obj[i].Y = 0 + rand() % 50;
						break;
					case 6:
						obj[i].X = -CREATE_ZONE / 2 + rand() % 50;
						obj[i].Y = -CREATE_ZONE / 2 + rand() % 50;
					case 7:
						obj[i].X = 0 + rand() % 50;
						obj[i].Y = -CREATE_ZONE / 2 + rand() % 50;
						break;
					case 8:
						obj[i].X = -CREATE_ZONE / 2 + rand() % 50;
						obj[i].Y = -CREATE_ZONE / 2 + rand() % 50;
						break;
					}
					p[j].life--;

					if (p[j].life < 1)
					{
						p[j].isDead = true;
						p[j].Ranking = RankingNumbering--;  //3,2,1�� ��ŷ ���� �ű� 
						p[j].Score = (int)SendToClientPacket.time; //�ð� �� ���ھ�ο���
					}
					if (p[j].Ranking == 1)
						isGameEnd = true;

					p[j].IsCollision = 1;
					p[j].invincibility = 1;
					invincibleTime[j] = clock();
				}
		}
		SendToClientPacket.object[i].X = obj[i].X;
		SendToClientPacket.object[i].Y = obj[i].Y;
		SendToClientPacket.object[i].Vx = obj[i].Vx;
		SendToClientPacket.object[i].Vy = obj[i].Vy;
	}
	for (int i = 0; i < PLAYERNUM; ++i)
	{
		SendToClientPacket.player[i].X = p[i].X;
		SendToClientPacket.player[i].Y = p[i].Y;
		SendToClientPacket.player[i].life = p[i].life;
		SendToClientPacket.player[i].num = p[i].num;
		SendToClientPacket.player[i].R = p[i].R;
		SendToClientPacket.player[i].G = p[i].G;
		SendToClientPacket.player[i].B = p[i].B;
		SendToClientPacket.player[i].IsCollision = p[i].IsCollision;
		
		SendToClientPacket.player[i].Ranking = p[i].Ranking;
		SendToClientPacket.player[i].Score = p[i].Score;
		SendToClientPacket.player[i].isGameEnd = isGameEnd;//Game���ΰŸ� �÷��̾� ���� ���� �پ˷��༭ ������. 
		
	}
}
// �ʱ�ȭ ��Ŷ ���� �Է�
void InitPacketServer()
{
	initPacket.time = 0.0f;
	for (int i = 0; i < PLAYERNUM; ++i)
	{
		initPacket.player[i].num = i;
		initPacket.player[i].R = 0.0f;
		initPacket.player[i].G = 0.0f;
		initPacket.player[i].B = 0.0f;
		initPacket.player[i].life = 3;
		if (i == 0)
		{
			initPacket.player[i].X = -20.0f;
			initPacket.player[i].Y = 0.0f;
			initPacket.player[i].R = 1.0f;

		}
		if (i == 1)
		{
			initPacket.player[i].X = 0.0f;
			initPacket.player[i].Y = 0.0f;
			initPacket.player[i].R = 1.0f;
			initPacket.player[i].G = 1.0f;
		}
		if (i == 2)
		{
			initPacket.player[i].X = 20.0f;
			initPacket.player[i].Y = 0.0f;
			initPacket.player[i].R = 1.0f;
			initPacket.player[i].B = 1.0f;
		}
	}
	for (int i = 0; i < MAX; ++i)
	{
		switch (POSIT_RANDOM)
		{
		case 1:
			initPacket.object[i].X = CREATE_ZONE / 2 + rand() % 50;
			initPacket.object[i].Y = 0 + rand() % 50;
			break;
		case 2:
			initPacket.object[i].X = CREATE_ZONE / 2 + rand() % 50;
			initPacket.object[i].Y = CREATE_ZONE / 2 + rand() % 50;
			break;
		case 3:
			initPacket.object[i].X = 0 + rand() % 50;
			initPacket.object[i].Y = CREATE_ZONE / 2 + rand() % 50;
			break;
		case 4:
			initPacket.object[i].X = -CREATE_ZONE / 2 + rand() % 50;
			initPacket.object[i].Y = CREATE_ZONE / 2 + rand() % 50;
			break;
		case 5:
			initPacket.object[i].X = -CREATE_ZONE / 2 + rand() % 50;
			initPacket.object[i].Y = 0 + rand() % 50;
			break;
		case 6:
			initPacket.object[i].X = -CREATE_ZONE / 2 + rand() % 50;
			initPacket.object[i].Y = -CREATE_ZONE / 2 + rand() % 50;
		case 7:
			initPacket.object[i].X = 0 + rand() % 50;
			initPacket.object[i].Y = -CREATE_ZONE / 2 + rand() % 50;
			break;
		case 8:
			initPacket.object[i].X = -CREATE_ZONE / 2 + rand() % 50;
			initPacket.object[i].Y = -CREATE_ZONE / 2 + rand() % 50;
			break;
		}
		switch (SPEED_RANDOM)
		{
		case 1:
			initPacket.object[i].Vx = ROOT3 * SPEED;
			initPacket.object[i].Vy = 1.0f * SPEED;
			break;
		case 2:
			initPacket.object[i].Vx = 1.0f* SPEED;
			initPacket.object[i].Vy = ROOT3* SPEED;
			break;				 
		case 3:					 
			initPacket.object[i].Vx = 0.0f* SPEED;
			initPacket.object[i].Vy = 2.0f* SPEED;
			break;
		case 4:
			initPacket.object[i].Vx = -1.0f* SPEED;
			initPacket.object[i].Vy = ROOT3* SPEED;
			break;				 
		case 5:					 
			initPacket.object[i].Vx = -ROOT3* SPEED;
			initPacket.object[i].Vy = 1.0f* SPEED;
			break;				 
		case 6:					 
			initPacket.object[i].Vx = -2.0f* SPEED;
			initPacket.object[i].Vy = 0.0f* SPEED;
			break;
		case 7:
			initPacket.object[i].Vx = -ROOT3* SPEED;
			initPacket.object[i].Vy = -1.0f* SPEED;
			break;				 
		case 8:					 
			initPacket.object[i].Vx = -1.0f* SPEED;
			initPacket.object[i].Vy = -ROOT3* SPEED;
			break;				 
		case 9:					 
			initPacket.object[i].Vx = 0.0f* SPEED;
			initPacket.object[i].Vy = -2.0f* SPEED;
			break;
		case 10:
			initPacket.object[i].Vx = 1.0f* SPEED;
			initPacket.object[i].Vy = -ROOT3* SPEED;
			break;				 
		case 11:				 
			initPacket.object[i].Vx = ROOT3* SPEED;
			initPacket.object[i].Vy = -1.0f* SPEED;
			break;				 
		case 12:				 
			initPacket.object[i].Vx = 2.0f* SPEED;
			initPacket.object[i].Vy = 0.0f* SPEED;
			break;
		}
		obj[i].Vx = initPacket.object[i].Vx;
		obj[i].Vy = initPacket.object[i].Vy;
		obj[i].X = initPacket.object[i].X;
		obj[i].Y = initPacket.object[i].Y;
		for (int i = 0; i < PLAYERNUM; ++i)
		{
			p[i].X = initPacket.player[i].X;
			p[i].Y = initPacket.player[i].Y;
			p[i].R = initPacket.player[i].R;
			p[i].G = initPacket.player[i].G;
			p[i].B = initPacket.player[i].B;
			p[i].life = initPacket.player[i].life;
		}
	}

	totalPlayTime = clock(); //�� �÷��� �ð� ��� ���ؼ� ���۽� �ʽð� ��ŸƮ 
	
}


int main(int argc, char* argv[])
{
	int retval;

	UpdateEvent[0] = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (UpdateEvent[0] == NULL) return 1;
	UpdateEvent[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (UpdateEvent[1] == NULL) return 1;
	UpdateEvent[2] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (UpdateEvent[2] == NULL) return 1;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));


	// listen()
	retval = listen(listen_sock, SOMAXCONN);


	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;
	//�ʱ�ȭ �� ����
	InitPacketServer();
	isGameEnd = false;   //Game �������� ���ӻ��� �ʱ�ȭ 

	while (1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			break;
		}
		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//������ ����
		hThread = CreateThread(NULL, 0, ConnectThread, (LPVOID)client_sock, 0, NULL);
		
		player_cnt++;
		cout << player_cnt << endl;
		if (hThread == NULL)
		{
			closesocket(client_sock);
		}
		else
		{
			CloseHandle(hThread);
		}
		//������ ���� ���� ���
		WaitForMultipleObjects(3, &hThread, TRUE, INFINITE);
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}
