#include "./sqlite_func.h"

#define ERR_MSG(msg) do{\
	printf("line = %d\n",__LINE__);\
	perror(msg);\
}while(0)

struct CliMsg
{
	struct sockaddr_in cin;
	int newfd;
	sqlite3* db;
};


void* recv_cli_msg(void* arg);

int main(int argc, const char *argv[])
{   int i;
	if( argc < 3)
	{
		printf("请输入IP和端口号\n");
		return -1;
	}

	//初始化数据库msg.db
	//创建员工信息数据库 
	//更新登陆状态 state = 0 未登陆 state = 1 已登录 
	sqlite3* db = NULL;
	i = sqlite_init(&db);
	if (i < 0)
	{
		printf("数据库初始化失败\n");
		return -1;
	}

	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))<0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}

	uint32_t ser_port =(uint32_t) atoi(argv[2]);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(ser_port);
	sin.sin_addr.s_addr = inet_addr(argv[1]);

	if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin)) < 0 )
	{
		ERR_MSG("bind");
		return -1;
	}
	printf("绑定成功\n");

	if(listen(sfd,20) < 0)
	{
		ERR_MSG("listen");
		return -1;
	}

	struct sockaddr_in cin;
	socklen_t addlen = sizeof(cin);

	int newfd = 0;
	pthread_t tid = 0;
	struct CliMsg info;

	while(1)
	{
		newfd = accept(sfd,(struct sockaddr*)&cin,&addlen);
		if(newfd < 0)
		{
			ERR_MSG("accept");
			return -1;
		}

		printf("********%s %d 连接成功********\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));

		info.cin = cin;
		info.newfd = newfd;
		info.db = db;

		if(pthread_create(&tid, NULL, recv_cli_msg, (void*)&info) < 0)
		{
			ERR_MSG("pthread_create");
			return -1;
		}	
	}
	close(sfd);
	return 0;
}

void* recv_cli_msg(void* arg)
{
	pthread_detach(pthread_self());

	struct CliMsg info = *(struct CliMsg*)arg;
	struct sockaddr_in cin = info.cin;
	int newfd = info.newfd;
	sqlite3* db = info.db;

	struct information buf;
	char msgback;
	int i;
	ssize_t res = 0;
	while(1)
	{
		bzero(&buf,sizeof(buf));
		res = recv(newfd, &buf, sizeof(buf), 0);
		if(res < 0)
		{
			ERR_MSG("recv");
			break;
		}
		else if(0 == res)
		{
			printf("********%s %d 客户端关闭********\n", inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
			char sql[128] = "update login set state=0";
			char* errmsg = NULL;
			if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
			{   
				printf("sqlite3_exec: %s    __%d__\n", errmsg, __LINE__);
			}
			break;
		}

		switch(buf.type)
		{
		case 'R':
			//在数据库登陆信息表 login 中增加新的用户
			i = do_insert_login(db,buf);
			bzero(&msgback,sizeof(msgback));
			if (i == 0)
			{
				printf("%s:注册成功\n",buf.name);
				msgback = '0';
				send(newfd,&msgback,sizeof(msgback),0);
			}
			else if ( i > 0 )
			{
				printf("%s:用户已注册，请直接登陆\n",buf.name);
				msgback = '1';
				send(newfd,&msgback,sizeof(msgback),0);
			}
			else
			{
				printf("%s:注册失败\n",buf.name);
				msgback = '9';
				send(newfd,&msgback,sizeof(msgback),0);
			}
			break;
		case 'L':
			//用户登陆
			i = do_select_login(db,buf);
			switch(i)
			{
			case 0:
				msgback = '0';
				printf("%s:登陆成功\n",buf.name);
				send(newfd,&msgback,sizeof(msgback),0);
				break;
			case 1:
				msgback = '1';
				printf("%s:用户名错误\n",buf.name);
				send(newfd,&msgback,sizeof(msgback),0);
				break;
			case 2:
				msgback = '2';
				printf("%s:密码错误\n",buf.name);
				send(newfd,&msgback,sizeof(msgback),0);
				break;
			case 3:
				msgback = '3';
				printf("%s:权限不对\n",buf.name);
				send(newfd,&msgback,sizeof(msgback),0);
				break;
			case 4:
				msgback = '4';
				printf("%s:用户已登陆\n",buf.name);
				send(newfd,&msgback,sizeof(msgback),0);
				break;
			case -1:
				msgback = '9';
				printf("%s:登陆失败\n",buf.name);
				send(newfd,&msgback,sizeof(msgback),0);
				break;
			}
			break;
		case 'Q':
			do_clean_state(db,buf);
			break;
		case 'D':
			i = do_delete(db,buf);
			switch(i)
			{
				case 0:
					msgback = '0';
					printf("删除成功\n");
					break;
				case 1:
					msgback = '1';
					printf("没有此人的信息\n");
					break;
				case -1:
					msgback = '9';
					printf("删除失败\n");
					break;

			}
			send(newfd,&msgback,sizeof(msgback),0);
			break;
		case 'a'://年龄
		case 'b'://密码
		case 'c'://电话
		case 'd'://地址
		case 'e'://月薪
		case 'f'://权限
			i = do_modify(db,buf);
			i ==0 ? (msgback = '0'): (msgback = '9');
			send(newfd,&msgback,sizeof(msgback),0);
			break;
		case 'S':
			i = do_search(db,&buf);
			send(newfd,&buf,sizeof(struct information),0);
			break;
	/*	case 'S':
			do_select_eng(db,&buf);
			send(newfd,&buf,sizeof(buf),0);
			break;
		case 'H':
			do_history(db,&buf,newfd);
			break;*/
		default:;
		}
	}
	close(newfd);
	pthread_exit(NULL);
}
