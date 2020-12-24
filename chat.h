#ifndef _TCP_CHAT
#define _TCP_CHAT

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define PORT 2500

//用户结构体
struct CHATMEMBER{
	char name[32];
	char passwd[32];
	int mark;//-1
	int flag;//是否在线
	int auth;//权限 普通用户为0 管理员为1
	int ban;//是否被禁言 0为正常 1为被禁言
};

#define MAX_MEMBER_NUM 100
struct CHATMEMBER member[MAX_MEMBER_NUM];
//C/S通信结构体
struct protocol{
	char name[32];
	char data[64];
	int cmd;
	int state;
};

//cmd
#define REGISTE 1
#define LOGIN 2
#define PUBLICCHAT 3
#define PRIVATECHAT 4
#define ONLINEMEM 5
#define LOGOUT 6
#define MODIFYPWD 7
#define DRIVEMEM 8
#define BANCHAT 9
#define LIFTBAN 10
//state
#define OP_OK 100
#define ONLINEMEM_OK 99
#define ONLINEMEM_OVER 98
#define NAME_EXIST 97
#define NAME_PWD_NMATCH 96
#define MEM_LOADED 95
#define MEM_NOT_REGIST 94
#define MEMURE 93
#define MODIFYPWD_OK 92
#define MEM_UNEXIST 91
#define NO_AUTHORITY 90
#define DRIVEMEM_OK 89
#define BANCHAT_OK 88

#endif
