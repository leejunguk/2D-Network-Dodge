#pragma once


#pragma comment(lib, "ws2_32")


#include <iostream>
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
//클라이언트 헤더 추가
#include <WinSock2.h>
#include <stdlib.h>
#include "Packets.h"
#include <time.h>

using namespace std;

#define TIME 33
#define MAX 200
#define WSIZE 700
#define UISIZE 50
#define CREATE_ZONE 750
#define BOUNDING_ZONE 850
#define PLAYER_SIZE 30.0f
#define BULLET_SIZE 5.0f
#define ROOT2 1.414f
#define ROOT3 1.732f
#define SPEED 1.3f
#define PLAYER_SPEED 6.0f

#define POSIT_RANDOM rand()%8 + 1
#define SPEED_RANDOM rand()%12 + 1

//클라이언트
#define SERVERIP	"114.204.69.195"
#define SERVERPORT 9000
#define BUFSIZE 4096

