#include "./sqlite_func.h"


#define ERR_MSG(msg) do{\
	printf("line = %d\n",__LINE__);\
	perror(msg);\
}while(0)

int main(int argc, const char *argv[])
{
	if( argc < 3)
	{
		printf("请输入IP和端口号\n");
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

	//连接服务器
	if(connect(sfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		ERR_MSG("connect");
		return -1;
	}
	printf("连接成功\n");

	struct information buf;
	char choice;
	while(1)
	{
		printf("***************************\n");
		printf("********1.注册（R）********\n");
		printf("********2.登陆（L）********\n");
		printf("********3.退出（Q）********\n");
		printf("***************************\n");
		printf("请输入选项:");
		scanf("%c",&choice);
		while(getchar()!=10);
		switch(choice)
		{
		case 'R':
			do_register(sfd);
			printf("请按ENTER键清屏\n");
			while(getchar()!=10);
			system("clear");
			break;
		case 'L':
			bzero(&buf,sizeof(buf));
			int i = do_login(sfd,&buf);
			if ( i == 0)
			{	
				printf("请按ENTER键进入下一界面");
				while(getchar()!=10);
				system("clear");
				do_func(sfd,&buf);
			}
			printf("请按ENTER键清屏\n");
			while(getchar()!=10);
			system("clear");
			break;
		case 'Q':
			buf.type = 'Q';
			send(sfd,&buf,sizeof(buf),0);
			exit(0);
			break;
		default:
			printf("请重新输入！\n");
			printf("请按ENTER键清屏\n");
			while(getchar()!=10);
			system("clear");
		}
	}
	close(sfd);
	return 0;
}


