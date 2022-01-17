#include "sqlite_func.h"

int sqlite_init(sqlite3** pdb)
{
	//创建数据库
	char* errmsg = NULL;
	char sql[256] = "";
	if(sqlite3_open("./msg.db",pdb) != SQLITE_OK)
	{
		printf("sqlite3_open:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}

	//创建登陆信息表
	bzero(sql,sizeof(sql));
	strcpy(sql,"create table if not exists login(name char primary key,\
		age char,passwd char,telephone char,address char,money char,level char,state int);");
	if(sqlite3_exec(*pdb,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}

	//更新登陆状态 state = 0 未登陆 state = 1 已登录
	bzero(sql,sizeof(sql));
	strcpy(sql,"update login set state=0;");
	if(sqlite3_exec(*pdb,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}
	printf("登陆状态加载成功\n");
	return 0;
}

int do_insert_login(sqlite3* db,struct information buf)
{
	char* errmsg = NULL;
	char sql[256] = "";
	char** presult = NULL;
	int row,colum;
	strcpy(sql,"select name from login;");
	if(sqlite3_get_table(db,sql,&presult,&row,&colum,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s 	__%d__\n", errmsg, __LINE__);
		return -1;
	}

	int i;
	for( i = 1; i < (row+1); i++)
	{
		if(strcmp(buf.name,presult[i]) == 0)
			return 1;
	}

	bzero(sql,sizeof(sql));
	sprintf(sql,"insert into login values(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%c\",0);",\
			buf.name,\
			buf.age,\
			buf.passwd,\
			buf.tel,\
			buf.add,\
			buf.money,\
			buf.level);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}
	sqlite3_free_table(presult);
	return 0;
}

int do_select_login(sqlite3* db,struct information buf)
	// 0 登陆成功；1 用户名错误；2 密码错误；3 权限不对； 4 用户已登陆；-1 登陆错误
{
	char sql[128] = "select * from login";
	char* errmsg = NULL;
	char **presult = NULL;
	int row, column;
	int line ,flag = 100;
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s 	__%d__\n", errmsg, __LINE__);
		return -1;
	}

	for(line = 1; line < (row+1); line++)
	{
		if(strcmp(buf.name,presult[8*line]) == 0)
		{
			if(strcmp(buf.passwd,presult[8*line+2]) == 0)
			{
				//printf("%c  %c",buf.check,*(presult[8*line+6]));
				if(buf.check == *(presult[8*line+6]))
				{
					if(*(presult[8*line+7]) == '0')
					{
						flag =  0;//登陆成功；
						bzero(sql,sizeof(sql));
						sprintf(sql,"update login set state=1 where name=\"%s\"",buf.name);
						if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
						{
							printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
							return -1;
						}
						break;
					}
					else
					{
						flag = 4;//用户已登陆；
						break;
					}
				}
				else
				{
					flag = 3;//权限不对
					break;
				}
			}
			else
			{
				flag =  2;//密码错误
				break;
			}
		}
	}
	if(line == (row+1))
		flag = 1;//用户名错误；
	sqlite3_free_table(presult);
	switch(flag)
	{
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
		return 3;
	case 4:
		return 4;
	}
	return 0;
}

int do_clean_state(sqlite3* db,struct information buf)//清空登陆状态
{
	char sql[256] = "";
	char* errmsg = NULL;
	sprintf(sql,"update login set state=0 where name=\"%s\";",buf.name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}
	return 0;
}

int do_delete(sqlite3* db, struct information buf)//删除员工信息
{
	char sql[128] = "select name from login";
	char* errmsg = NULL;
	char **presult = NULL;
	int row, column;
	int line ,flag = 100;
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s 	__%d__\n", errmsg, __LINE__);
		return -1;
	}
	for(line = 1; line < (row+1);line++)
	{
		if(strcmp(buf.name,presult[line])==0)
		{
			bzero(sql,sizeof(sql));
			sprintf(sql,"delete from login where name=\"%s\";",buf.name);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
			{
				printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
				return -1;
			}
			return 0;
		}
	}
	sqlite3_free_table(presult);
	return 1;
}

int do_modify(sqlite3* db,struct information buf)//修改员工信息
{
	char sql[256] = "";
	char* errmsg = NULL;
	switch(buf.type)
	{
	case 'a':
		sprintf(sql,"update login set age=\"%s\" where name=\"%s\";",buf.age,buf.name);
		break;
	case 'b':
		sprintf(sql,"update login set age=\"%s\" where name=\"%s\";",buf.passwd,buf.name);
		break;
	case 'c':
		sprintf(sql,"update login set age=\"%s\" where name=\"%s\";",buf.tel,buf.name);
		break;
	case 'd':
		sprintf(sql,"update login set age=\"%s\" where name=\"%s\";",buf.add,buf.name);
		break;
	case 'e':
		sprintf(sql,"update login set age=\"%s\" where name=\"%s\";",buf.money,buf.name);
		break;
	case 'f':
		sprintf(sql,"update login set age=\"%c\" where name=\"%s\";",buf.level,buf.name);
		break;
	default:;
	}
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}
	return 0;
}
int do_search(sqlite3* db,struct information* buf)//查询员工信息
{
	char sql[128] = "select * from login";
	char* errmsg = NULL;
	char **presult = NULL;
	int row, column;
	int line;
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s   __%d__\n", errmsg, __LINE__);
		return -1;
	}
	for(line = 1; line < (row+1);line++)
	{
		if(strcmp(buf->name,presult[line * 8])==0)
		{
			strcpy(buf->age,presult[line*8+1]);
			strcpy(buf->passwd,presult[line*8+2]);
			strcpy(buf->tel,presult[line*8+3]);
			strcpy(buf->add,presult[line*8+4]);
			strcpy(buf->money,presult[line*8+5]);
			buf->level = *(presult[line*8+6]);
			return 0;                                                                
		}
	}
	sqlite3_free_table(presult);
	buf->level = '9';
	return -1;
}

#if 0
int do_select_eng(sqlite3* db,struct information* pbuf)
{
	char sql[256] = "select * from eng";
	char **presult = NULL;
	int row, column;
	char* errmsg = NULL;
	int line;
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s 	__%d__\n", errmsg, __LINE__);
		return -1;
	}
	for(line = 1; line < (row+1); line++)
	{
		if(strcasecmp(pbuf->eng,presult[2*line]) == 0)
		{
			bzero(pbuf->chn,sizeof(pbuf->chn));
			strcpy(pbuf->chn,presult[2*line+1]);//翻译完成；
			break;
		}
	}
	if( line == (row+1))
	{
		bzero(pbuf->chn,sizeof(pbuf->chn));
		strcpy(pbuf->chn,"not found");//词库中没有此单词；
	}
	sqlite3_free_table(presult);

	bzero(sql,sizeof(sql));
	time_t t1 = time(NULL);
	struct tm* t2 = NULL;
	t2 = localtime(&t1);
	//char time1[64] = "";
	sprintf(pbuf->time1,"%4d-%02d-%02d %02d:%02d:%02d",t2->tm_year+1900, t2->tm_mon+1, t2->tm_mday,\
			t2->tm_hour, t2->tm_min, t2->tm_sec);
	sprintf(sql,"insert into history values(\"%s\",\"%s\",\"%s\",\"%s\");",\
			pbuf->name,pbuf->eng,pbuf->chn,pbuf->time1);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec:%s __%d__\n",errmsg,__LINE__);
		return -1;
	}	
	return 0;
}

int do_history(sqlite3* db,struct information* pbuf,int newfd)
{
	char sql[256] = "select * from history";
	char **presult = NULL;
	int row, column;
	char* errmsg = NULL;
	int line;
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s 	__%d__\n", errmsg, __LINE__);
		return -1;
	}
	for(line = 1; line < (row+1); line++)
	{
		if(strcmp(pbuf->name,presult[4*line]) == 0)
		{
			bzero(pbuf->eng,sizeof(pbuf->eng));
			bzero(pbuf->chn,sizeof(pbuf->chn));
			bzero(pbuf->time1,sizeof(pbuf->time1));
			strcpy(pbuf->eng,presult[4*line+1]);
			strcpy(pbuf->chn,presult[4*line+2]);
			strcpy(pbuf->time1,presult[4*line+3]);
			send(newfd,pbuf,sizeof(struct information),0);
		}
	}
	pbuf->type = 'O';
	send(newfd,pbuf,sizeof(struct information),0);
	printf("%s:历史数据发送结束\n",pbuf->name);
	return 0;
}
#endif

int do_register(int sfd)
{
	struct information buf;
	char msgback;
	ssize_t res = 0;

	buf.type = 'R';
	while(1)
	{
		printf("是否注册为管理员？ Y/N >>");
		scanf("%c",&buf.level);
		if (buf.level == 'Y' || buf.level == 'y')
		{
			buf.level = '0';
			break;
		}
		else if (buf.level == 'N' || buf.level == 'n')
		{
			buf.level = '1';
			break;
		}
		else 
		{
			printf("输入错误\n");
		}
	}
	printf("请输入注册名>>");
	scanf("%s",buf.name);
	while(getchar()!=10);

	printf("请输入年龄>>");
	scanf("%s",buf.age);
	while(getchar()!=10);

	printf("请输入密码(八位以内)>>");
	scanf("%s",buf.passwd);
	while(getchar()!=10);

	printf("请输入电话>>");
	scanf("%s",buf.tel);
	while(getchar()!=10);

	printf("请输入住址>>");
	scanf("%s",buf.add);
	while(getchar()!=10);

	printf("请输入月薪>>");
	scanf("%s",buf.money);
	while(getchar()!=10);

	send(sfd,&buf,sizeof(buf),0);

	bzero(&msgback,sizeof(msgback));
	res = recv(sfd,&msgback,sizeof(msgback),0);
	if(res < 0)
	{
		printf("recv failed __%d__\n",__LINE__);
		return -1;
	}
	switch(msgback)
	{
	case '0':
		printf("注册成功\n");
		break;
	case '1':
		printf("用户已注册，请直接登陆\n");
		break;
	case '9':
		printf("注册失败\n");
		break;
	}

	return 0;
}

int do_login(int sfd,struct information* pbuf)
{
	char msgback;
	ssize_t res = 0;

	pbuf->type = 'L';
	while(1)
	{
		printf("是否以管理员身份登陆？ Y/N >>");
		scanf("%c",&pbuf->check);
		while(getchar()!=10);
		if(pbuf->check == 'Y' || pbuf->check == 'y')
		{
			pbuf->check = '0';
			break;
		}
		else if(pbuf->check == 'N' || pbuf->check == 'n')
		{
			pbuf->check = '1';
			break;
		}
		else
		{
			printf("输入错误\n");
		}
	}
	bzero(pbuf->name,sizeof(pbuf->name));
	printf("请输入登陆名>>");
	scanf("%s",pbuf->name);
	while(getchar()!=10);

	bzero(pbuf->passwd,sizeof(pbuf->passwd));
	printf("请输入密码>>");
	scanf("%s",pbuf->passwd);
	while(getchar()!=10);


	send(sfd,pbuf,sizeof(struct information),0);
	res = recv(sfd,&msgback,sizeof(msgback),0);
	if(res < 0)
	{
		printf("recv failed __%d__\n",__LINE__);
		return -1;
	}
	switch(msgback)
	{
	case '0':
		printf("登陆成功\n");
		return 0;
		break;
	case '1':
		printf("用户名错误\n");
		return -1;
		break;
	case '2':
		printf("密码错误\n");
		return -1;
		break;
	case '3':
		printf("权限不对\n");
		return -1;
		break;
	case '4':
		printf("用户已登陆\n");
		return -1;
		break;
	case '9':
		printf("登陆失败\n");
		return -1;
		break;
	}
	return -2;
}

int do_func(int sfd,struct information* pbuf)
{
	char choice,msgback;
	struct information newbuf;
	ssize_t res = 0;
	while(1)                                                                                           
	{
		printf("*****************************\n");
		printf("********1.添加信息(A)********\n");
		printf("********2.删除信息(D)********\n");
		printf("********3.修改信息(C)********\n");
		printf("********4.查询信息(S)********\n");
		printf("********5.返回上级(Q)********\n");
		printf("当前用户：%s\n",pbuf->name);
		printf("*****************************\n");

		printf("请输入选项>>");
		scanf("%c",&choice);
		while(getchar()!=10);
		memset(&newbuf,0,sizeof(struct information));
		switch(choice)
		{
		case 'A'://添加信息
			if (pbuf->check == '1')
			{
				printf("无此权限\n");
				break;
			}
			do_register(sfd);
			break;
		case 'D'://删除信息
			if (pbuf->check == '1')
			{
				printf("无此权限\n");
				break;
			}
			newbuf.type = 'D';
			printf("请输入要删除的员工的姓名>>");
			scanf("%s",newbuf.name);
			while(getchar()!=10);

			//printf("%s\n",newbuf.name);
			//printf("%s\n",pbuf->name);

			if (strcmp(newbuf.name,pbuf->name) == 0)
			{
				printf("无法删除当前已登陆账号\n");
				break;
			}
			send(sfd,&newbuf,sizeof(struct information),0);
			res = recv(sfd,&msgback,sizeof(char),0);
			if(res < 0)
			{
				printf("recv failed __%d__\n",__LINE__);
				return -1;
			}
			switch(msgback)
			{
			case '0':
				printf("删除成功\n");
				break;
			case '1':
				printf("没有此人的信息\n");
				break;
			default:
				printf("删除失败\n");
			}
			break;
		case 'C':// 修改信息 年龄a 密码b 电话c 地址d 月薪e 权限f
			if(pbuf->check == '1')//普通员工
			{
				strcpy(newbuf.name,pbuf->name);
				printf("请选择需要修改的内容： b/密码 c/电话 d/地址 >>");
				scanf("%c",&newbuf.type);
				while(getchar()!=10);
				switch(newbuf.type)
				{
				case 'b':
					printf("请输入新的密码>>");
					scanf("%s",newbuf.passwd);
					while(getchar()!=10);
					break;
				case 'c':
					printf("请输入新的电话>>");
					scanf("%s",newbuf.tel);
					while(getchar()!=10);
					break;
				case 'd':
					printf("请输入新的地址>>");
					scanf("%s",newbuf.add);
					while(getchar()!=10);
					break;
				default:
					printf("没有此权限\n");
				}
				send(sfd,&newbuf,sizeof(struct information),0);
				res = recv(sfd,&msgback,sizeof(char),0);
				if (res < 0)
				{
					printf("recv failed __%d__\n",__LINE__);
					return -1;
				}
				if (msgback == '0')
				{
					printf("修改成功\n");
				}
				else
				{
					printf("修改失败\n");
				}
			}
			else if( pbuf->check == '0')//管理员 修改信息 年龄a 密码b 电话c 地址d 月薪e 权限f  
			{
				printf("请输入需要修改的员工姓名>>");
				scanf("%s",newbuf.name);
				while(getchar()!=10);
				printf("请选择需要修改的内容： a/年龄 b/密码 c/电话 d/地址 e/月薪 f/权限>>"); 
				scanf("%c",&newbuf.type);
				while(getchar()!=10);
				switch(newbuf.type)
				{
				case 'a':
					printf("请输入新的年龄>>");
					scanf("%s",newbuf.age);
					while(getchar()!=10);
					break;
				case 'b':
					printf("请输入新的密码>>");
					scanf("%s",newbuf.passwd);
					while(getchar()!=10);
					break;
				case 'c':
					printf("请输入新的电话>>");
					scanf("%s",newbuf.tel);
					while(getchar()!=10);
					break;
				case 'd':
					printf("请输入新的地址>>");
					scanf("%s",newbuf.add);
					while(getchar()!=10);
					break;
				case 'e':
					printf("请输入新的月薪>>");
					scanf("%s",newbuf.money);
					while(getchar()!=10);
					break;
				case 'f':
					printf("请输入新的权限>>");
					scanf("%c",&newbuf.level);
					while(getchar()!=10);
					break;

				default:
					printf("没有此权限\n");
				}
				send(sfd,&newbuf,sizeof(struct information),0);
				res = recv(sfd,&msgback,sizeof(char),0);
				if (res < 0)
				{
					printf("recv failed __%d__\n",__LINE__);
					return -1;
				}
				if (msgback == '0')
				{
					printf("修改成功\n");
				}
				else
				{
					printf("修改失败\n");
				}
			}
			break;
		case 'S'://查询信息
			newbuf.type = 'S';
			if(pbuf->check == '1')
			{
				strcpy(newbuf.name,pbuf->name);
			}
			else if (pbuf->check == '0')
			{
				printf("请输入员工的姓名>>");
				scanf("%s",newbuf.name);
				while(getchar()!=10);
			}
			send(sfd,&newbuf,sizeof(struct information),0);
			res = recv(sfd,&newbuf,sizeof(struct information),0);
			if (res < 0)
			{
				printf("recv failed __%d__\n",__LINE__);
				return -1;
			}
			if(newbuf.level == '9')
			{
				printf("没有此人的消息\n");
				break;
			}
			putchar(10);
			printf("以下为查询到的信息：\n");
			printf("姓名:%s 年龄:%s 密码:%s 电话:%s 地址:%s 月薪:%s 权限:%c\n",\
					newbuf.name,\
					newbuf.age,\
					newbuf.passwd,\
					newbuf.tel,\
					newbuf.add,\
					newbuf.money,\
					newbuf.level);
			putchar(10);
			break;
		case 'Q'://返回上一级
			return 0;
			break;
		default:
			printf("请重新输入\n");           
			printf("请按ENTER键清屏\n");
			while(getchar()!=10);
			system("clear");
		}
	}
	return 0;
}








