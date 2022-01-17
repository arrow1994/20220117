#ifndef SQLITE_FUNC_H
#define SQLITE_FUNC_H

#include <stdio.h>           
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>

struct information{
	char type;       //信息类型 R注册  L登陆  Q退出 A增加 D删除 C更改 L查询
	char name[10];   //姓名
	char age[5];    //年龄 
	char passwd[10]; //密码
	char tel[15];   //电话
	char add[128];   //住址
	char money[10];  //月薪
	char level;     //权限  0管理员 1员工
	char check;     //权限检查
}__attribute__((packed));

int sqlite_init(sqlite3** pdb);
int do_insert_login(sqlite3* db,struct information buf);//0，注册成功；-1 注册失败；1 已经注册；
int do_select_login(sqlite3* db,struct information buf);
int do_clean_state(sqlite3* db,struct information buf);
int do_select_eng(sqlite3* db,struct information* pbuf);
int do_history(sqlite3* db,struct information* pbuf,int newfd);

int do_delete(sqlite3* db,struct information buf);
int do_modify(sqlite3* db,struct information buf);
int do_search(sqlite3* db,struct information* buf);

int do_register(int sfd);
int do_login(int sfd,struct information* pbuf);
int do_func(int sfd,struct information* pbuf);

#endif
