#include "chat.h"
int sockfd;
int serlen;
struct sockaddr_in server_addr;
pthread_t thread;
int login_t=-1;

int registe(int fd);//注册
int login(int fd);//登录
int logout(int fd);//退出
void public_chat(int fd);//群聊
void private_chat(int fd);//私聊
void list_member_online(sockfd);//在线用户
void modify_pwd(int fd);//修改密码
int drive_member(int fd);//踢人
int banchat(int fd);//禁言
int liftban(int fd);//解禁

void *func(void *arg){
	struct protocol *msg;
	char buf[128]={0};
	while(1){
		if(login_t!=1){
			continue;
		}
		memset(buf,0,sizeof(buf));
		int len=recv(sockfd,buf,sizeof(buf),0);
		if(len<=0){
			close(sockfd);
			return;
		}
		msg=(struct protocol *)buf;
		if((msg->state==ONLINEMEM_OK)&&(msg->cmd==ONLINEMEM)){
			printf("[%s]\n",msg->name);
			continue;
		}
		if((msg->state==ONLINEMEM_OVER)&&(msg->cmd==ONLINEMEM)){
			continue;
		}
		buf[len]='\0';
		printf("%s\n",buf);
	}
}

int main(int argc,char **argv){
	struct protocol msg;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket error\n");
		exit(1);
	}
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(PORT);
	inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr.s_addr);
	serlen=sizeof(server_addr);
	if(connect(sockfd,(struct sockaddr*)&server_addr,serlen)==-1){
		perror("connect error\n");
		exit(1);
	}
	pthread_create(&thread,NULL,func,NULL);
	int sel,ret;
	while(1){
		system("clear");
		if(login_t==-1){
			printf("===Welcome ChatRoom===\n");
			printf("       1.注册         \n");
			printf("       2.登录         \n");
			printf("       3.修改密码     \n");
		}else if(login_t==1){
			printf("===Welcome ChatRoom===\n");
			printf("       4.群聊         \n");
			printf("       5.私聊         \n");
			printf("       6.在线列表     \n");
			printf("       7.踢人         \n");
			printf("       8.禁言         \n");
			printf("       9.解禁         \n");
		}
		printf("       0.退出      \n ");
		printf("——————————————————\n");
		fflush(stdin);
		printf("input your selection:\n");
		scanf("%d",&sel);
		if(sel==0){
			break;
		}
		switch(sel){
			case 1:
			    registe(sockfd);
				break;
			case 2:
			    ret=login(sockfd);
				break;
			case 3:
			    modify_pwd(sockfd);
				break;
			case 4:
			    public_chat(sockfd);
				break;
			case 5:
			    private_chat(sockfd);
				break;
			case 6:
			    list_member_online(sockfd);
				break;
			case 7:
			    drive_member(sockfd);
				break;
			case 8:
			    banchat(sockfd);
				break;
			case 9:
			    liftban(sockfd);
				break;
			case 0:
			    logout(sockfd);
				break;
			default:
			    break;
		}
		if(sel==0){
			exit(0);
		}
	}
}

//注册
int registe(int fd){
	struct protocol msg,msgback;
	msg.cmd=REGISTE;
	printf("input name:\n");
	scanf("%s",msg.name);
	printf("input password:\n");
	scanf("%s",msg.data);
	send(sockfd,&msg,sizeof(msg),0);
	recv(sockfd,&msgback,sizeof(msgback),0);
	if(msgback.state!=OP_OK){
		printf("name had existed,please try again!\n");
		getchar();
		getchar();
		return -1;
	}else{
		printf("register success!\n");
		getchar();
		getchar();
		return 0;
	}
}
//登录
int login(int fd){
	struct protocol msg,msgback;
	msg.cmd=LOGIN;
	printf("input name:\n");
	scanf("%s",msg.name);
	printf("input password:\n");
	scanf("%s",msg.data);
	send(sockfd,&msg,sizeof(msg),0);
	recv(sockfd,&msgback,sizeof(msgback),0);
	if(msgback.state!=OP_OK){
		if(msgback.state==MEM_LOADED){
			printf("member had loaded");
			getchar();
			getchar();
			login_t=1;
			return MEM_LOADED;
		}
		else{
			printf("name and password unmatch!please try again!\n");
		}
		getchar();
		getchar();
		login_t=-1;
		return NAME_PWD_NMATCH;
	}else{
		printf("login success!\n");
		getchar();
		getchar();
		login_t=1;
		return OP_OK;
	}
}
//退出
int logout(int fd){
	close(fd);
	login_t=-1;
	printf("You already exit!\n");
}
//群聊
void public_chat(int fd){
	struct protocol msg;
	msg.cmd=PUBLICCHAT;
	printf("say:\n->");
	scanf("%s",msg.data);
    send(fd,&msg,sizeof(msg),0);
}

//私聊
void private_chat(int fd){
	struct protocol msg;
	msg.cmd=PRIVATECHAT;
	printf("input name you want to talk:\n");
	scanf("%s",msg.name);
	printf("say:\n->");
	scanf("%s",msg.data);
	send(fd,&msg,sizeof(msg),0);
}
//在线列表
void list_member_online(sockfd){
	struct protocol msg;
	msg.cmd=ONLINEMEM;
	send(sockfd,&msg,sizeof(msg),0);
	getchar();
	getchar();
}
//修改密码
void modify_pwd(int fd){
	struct protocol msg,msgback;
	msg.cmd=MODIFYPWD;
	printf("input name:\n");
	scanf("%s",msg.name);
	printf("input new password:\n");
	scanf("%s",msg.data);
	send(sockfd,&msg,sizeof(msg),0);
	recv(sockfd,&msgback,sizeof(msgback),0);
	if(msgback.state==MEMURE){
		printf("modify password fail,please try again!\n");
		getchar();
		getchar();
		return -1;
	}else{
		printf("modify password success!\n");
		getchar();
		getchar();
		return 0;
	}
}

//禁言
int banchat(int fd){
	struct protocol msg,msgback;
	msg.cmd=BANCHAT;
	char buf[128]={0};
	printf("input your name:\n");
	scanf("%s",msg.name);
	printf("input name to want to banchat:\n");
	scanf("%s",msg.data);
	send(sockfd,&msg,sizeof(msg),0);
	//recv(sockfd,&msgback,sizeof(msgback),0);
	//printf("%s\n",msgback.data);
	recv(sockfd,buf,sizeof(buf),0);

/*
	if(msgback.state==MEM_UNEXIST){
		printf("member: %s not exist!\n",msg.data);
	}else if(msgback.state==NO_AUTHORITY){
		printf("you have no authority!\n");
	}else{
		printf("you banchat %s success!\n",msg.data);
	}
	*/
	getchar();
	getchar();
	return 0;
}
//解禁
int liftban(int fd){
	struct protocol msg,msgback;
	msg.cmd=LIFTBAN;
	char buf[128]={0};
	printf("input your name:\n");
	scanf("%s",msg.name);
	printf("input name to want to liftban:\n");
	scanf("%s",msg.data);
	send(sockfd,&msg,sizeof(msg),0);
	//recv(sockfd,&msgback,sizeof(msgback),0);
	//printf("%s\n",msgback.data);
	recv(sockfd,buf,sizeof(buf),0);
	getchar();
	getchar();
	return 0;
}

//踢人
int drive_member(int fd){
	struct protocol msg,msgback;
	char buf[128]={0};
	msg.cmd=DRIVEMEM;
	printf("input your name:\n");
	scanf("%s",msg.name);
	printf("input name to want to drive:\n");
	scanf("%s",msg.data);
	send(sockfd,&msg,sizeof(msg),0);
	recv(sockfd,buf,sizeof(buf),0);
	/*
	if(msgback.state==MEM_UNEXIST){
		printf("member: %s not exist!\n",msg.data);
	}else if(msgback.state==NO_AUTHORITY){
		printf("you have no authority!\n");
	}else{
		printf("you drive %s success!\n",msg.data);
	}
	*/
	getchar();
	getchar();
	return 0;
}


