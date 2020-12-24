#include "chat.h"

struct CHATMEMBER member[MAX_MEMBER_NUM];
int tag=1;

int find_member(char *name);//按用户名查找（注册）
int find_member_online(int sockfd,int *index,struct protocol *msg);//按用户名查找（登录）
void registe(int sockfd,int *index,struct protocol *msg);//注册
void login(int sockfd,int *index,struct protocol *msg);//登录
void exit_member(int index);//退出登录
int add_member(int sockfd,struct protocol *msg);//将用户添加到注册列表
void list_member_online(int index);//在线用户列表
void public_chat(int index,struct protocol *msg);//群聊
void private_chat(int index,struct protocol *msg);//私聊
void modify_pwd(int sockfd,int *index,struct protocol *msg);//修改密码
int drive_member(int sockfd,int *index,struct protocol *msg);//踢人 
int banchat(int sockfd,int *index,struct protocol *msg);//禁言
int liftban(int sockfd,int *index,struct protocol *msg);//解禁


void *func(void *arg){
	int sockfd=*((int*)arg);
	int index=-1;
	struct protocol msg;
	free(arg);
	while(1){
		int len=recv(sockfd,&msg,sizeof(msg),0);
		if(len<=0){
			printf("member: %s offline!\n",member[index].name);
			exit_member(index);
			close(sockfd);
			return;
		}
		switch(msg.cmd){
			case REGISTE:
			    registe(sockfd,&index,&msg);
				break;
			case LOGIN:
			    login(sockfd,&index,&msg);
				break;
			case PUBLICCHAT:
			    public_chat(index,&msg);
				break;
			case PRIVATECHAT:
			    private_chat(index,&msg);
				break;
			case ONLINEMEM:
			    list_member_online(index);
				break;
			case MODIFYPWD:
			    modify_pwd(sockfd,&index,&msg);
				break;
			case DRIVEMEM:
			    drive_member(sockfd,&index,&msg);
				break;
			case BANCHAT:
			    banchat(sockfd,&index,&msg);
				break;
			case LIFTBAN:
			    liftban(sockfd,&index,&msg);
				break;
			default:
			    break;
		}
	}
}
int main(int argc,char **argv){
	int sockfd,confd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	pthread_t thread;
	int serlen,clilen;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("sockfd error\n");
		exit(1);
	}
	bzero(&server_addr,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(PORT);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serlen=sizeof(server_addr);
	if(bind(sockfd,(struct sockaddr*)&server_addr,serlen)==-1){
		perror("bind error\n");
		exit(1);
	}
	if(listen(sockfd,10)==-1){
		perror("listen error\n");
		exit(1);
	}

	for(int i=0;i<MAX_MEMBER_NUM;i++){
		member[i].mark=-1;
		member[i].flag=-1;
		member[i].auth=0;
		member[i].ban=0;
	}
	int *arg;
	while(1){
	  	if((confd=accept(sockfd,(struct sockaddr*)&client_addr,(socklen_t*)&clilen))<0){
             perror("accept error!\n");
             exit(1);
         }
		 printf("client-ip:%s\n",inet_ntoa(client_addr.sin_addr));
		 arg=malloc(sizeof(int));
		 *arg=confd;
		 pthread_create(&thread,NULL,func,(void*)arg);
	}
	close(confd);
	close(sockfd);
}


//解禁
int liftban(int sockfd,int *index,struct protocol *msg){
	int self,target;
	//struct protocol msg_back;
	char buf[128]={0};
	self=find_member(msg->name);
	target=find_member(msg->data);
	if(self<0||target<0){
		printf("liftban member %s chat fail!\n",msg->data);
		sprintf(buf,"liftban member %s chat fail!\n",msg->data);
		send(member[self].mark,buf,sizeof(buf),0);
		return 0;
	}
	if(member[*index].auth==0){
		printf("member: %s has no authority to liftban member chat!\n",msg->name);
		sprintf(buf,"member: %s has no authority to liftban member chat!\n",msg->name);
		send(member[self].mark,buf,sizeof(buf),0);
		return 0;
	}
	if((member[*index].auth==1)&&(member[target].ban==1)){
		member[target].ban=0;
		printf("member: %s can chat!\n",msg->data);
		sprintf(buf,"member: %s can chat!\n",msg->data);
		send(member[self].mark,buf,sizeof(buf),0);
		//send(member[target].mark,buf,sizeof(buf),0);
		return 0;
	}


}

//禁言
int banchat(int sockfd,int *index,struct protocol *msg){
	int self,target;
	//struct protocol msg_back;
	char buf[128]={0};
	//msg_back.cmd=BANCHAT;
	self=find_member(msg->name);
	target=find_member(msg->data);
	if(self<0||target<0){
		//msg_back.state=MEM_UNEXIST;
		printf("ban member %s chat fail!\n",msg->data);
		//sprintf(msg_back.data,"ban member %s chat fail!\n",msg->data);
		//send(member[self].mark,&msg_back,sizeof(msg_back),0);
		sprintf(buf,"ban member %s chat fail!\n",msg->data);
		send(member[self].mark,buf,sizeof(buf),0);
		return 0;
	}
	if(member[*index].auth==0){
		//msg_back.state=NO_AUTHORITY;
		printf("member: %s has no authority to ban member chat!\n",msg->name);
		//sprintf(msg_back.data,"member: %s has no authority to ban member chat!\n",msg->name);
		//send(member[self].mark,&msg_back,sizeof(msg_back),0);
		sprintf(buf,"member: %s has no authority to ban member chat!\n",msg->name);
		send(member[self].mark,buf,sizeof(buf),0);
		return 0;
	}
	if(member[*index].auth==1){
		member[target].ban=1;
		//msg_back.state=BANCHAT_OK;
		printf("member: %s can't chat!\n",msg->data);
		//sprintf(msg_back.data,"member: %s can't chat!\n",msg->data);
		//send(member[self].mark,&msg_back,sizeof(msg_back),0);
		sprintf(buf,"member: %s can't chat!\n",msg->data);
		send(member[self].mark,buf,sizeof(buf),0);
		//send(member[target].mark,buf,sizeof(buf),0);
		return 0;
	}
	
}
//踢人
int drive_member(int sockfd,int *index,struct protocol *msg){
	int self,target;
	struct protocol msg_back;
	char buf[128]={0};
	//msg_back.cmd=DRIVEMEM;
	self=find_member(msg->name);
	target=find_member(msg->data);
	if(self<0||target<0){
		//msg_back.state=MEM_UNEXIST;
		printf("drive member %s fail!\n",msg->data);
		sprintf(buf,"drive member %s fail!\n",msg->data);
		send(member[self].mark,buf,sizeof(buf),0);
		return 0;
	}
	if(member[self].auth==0){
		//msg_back.state=NO_AUTHORITY;
		printf("member: %s has no authority to drive member!\n",msg->name);
		sprintf(buf,"member: %s has no authority to drive member!\n",msg->name);
		send(member[self].mark,buf,sizeof(buf),0);
		return 0;
	}
	if(member[self].auth==1){
		exit_member(target);
		//msg_back.state=DRIVEMEM_OK;
		printf("member: %s had driven %s\n",msg->name,msg->data);
		sprintf(buf,"member: %s had driven %s!\n",msg->name,msg->data);
		send(member[self].mark,buf,sizeof(buf),0);
		//send(member[target].mark,buf,sizeof(buf),0);
		return 0;
	}
}

//按用户名查找位置（注册）
int find_member(char *name){
	int i;
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].flag==-1){//用户未注册
			continue;
		}
		if(strcmp(name,member[i].name)==0){
			return i;
		}
    }
	return -1;
}

//按用户名查找位置（登录）
int find_member_online(int sockfd,int *index,struct protocol *msg){
	int i;
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].flag==-1){
			continue;
		}
		if((strcmp(msg->name,member[i].name)==0)&&(strcmp(msg->data,member[i].passwd)==0)){
			if(member[i].mark==-1){
				member[i].mark=sockfd;
				*index=i;
				return OP_OK;
			}else{
				printf("member: %s had login\n",member[i].name);
				return MEM_LOADED;
			}
		}
	}
	return NAME_PWD_NMATCH;
}
//注册
void registe(int sockfd,int *index,struct protocol *msg){
	int target;
	struct protocol msg_back;
	msg_back.cmd=REGISTE;
	target=find_member(msg->name);//找到目标
	if(target==-1){//用户未注册
		*index=add_member(sockfd,msg);
		member[*index].flag=1;
		msg_back.state=OP_OK;
		printf("member: %s register!Welcome!\n",msg->name);
		send(sockfd,&msg_back,sizeof(msg_back),0);
		return;
	}else{
		msg_back.state=NAME_EXIST;
		printf("member: %s exist!\n",msg->name);
		send(sockfd,&msg_back,sizeof(msg_back),0);
		return;
	}
}

//登录
void login(int sockfd,int *index,struct protocol *msg){
	int i,ret;
	char buf[128]={0};
	struct protocol msg_back;
	msg_back.cmd=LOGIN;
	ret=find_member_online(sockfd,index,msg);
	if(ret!=OP_OK){
		msg_back.state=ret;
		if(ret==MEM_LOADED){
			strcpy(msg_back.data,"login repeat!\n");
		}else{
			strcpy(msg_back.data,"name and password unmatch!\n");
		}
		printf("member: %s login fail!\n",msg->name);
		send(sockfd,&msg_back,sizeof(msg_back),0);
		return;
	}else{
		member[*index].mark=sockfd;
		msg_back.state=OP_OK;
		strcpy(msg_back.data,"login success\n");
		printf("member: %s login success!\n",msg->name);
		send(member[*index].mark,&msg_back,sizeof(msg_back),0);
	}
	sprintf(buf,"member: %s login!\n",member[*index].name);
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].mark!=-1){
			send(member[i].mark,buf,sizeof(buf),0);
		}
	}
}

//退出
void exit_member(int index){
	int i;
	char buf[128]={0};
	if(index<0){
		return;
	}
	member[index].mark=-1;
	//member[index].auth=0;
	sprintf(buf,"member: %s exit!\n",member[index].name);
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].mark==-1){
			continue;
		}
		send(member[i].mark,buf,sizeof(buf),0);
	}
	return;
}
//将用户添加到注册列表
int add_member(int sockfd,struct protocol *msg){
	int i,index=-1;
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].flag==-1){
			if(i==0){
				member[i].auth=1;
			}
			member[i].flag=1;
			strcpy(member[i].name,msg->name);
			strcpy(member[i].passwd,msg->data);
			printf("member:%s register!Welcome!\n",msg->name);
			index=i;
			return index;
		}
	}
	return index;
}
//在线用户列表
void list_member_online(int index){
	int i;
	struct protocol msg;
	char buf[128]={0};
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].mark==-1){
			continue;
		}
		memset(&msg,0,sizeof(msg));
		msg.cmd=ONLINEMEM;
		msg.state=ONLINEMEM_OK;
		strcpy(msg.name,member[i].name);
		sprintf(buf,"[%s] online\n",member[i].name);
		send(member[index].mark,buf,sizeof(buf),0);
	}
	msg.cmd=ONLINEMEM;
	msg.state=ONLINEMEM_OVER;
	send(member[index].mark,&msg,sizeof(msg),0);
}

//群聊
void public_chat(int index,struct protocol *msg){
	int i;
	char buf[128]={0};
	char buf1[128]={0};
	if(member[index].ban==1){
		sprintf(buf,"member: %s can't chat!\n",member[index].name);
		send(member[index].mark,buf,sizeof(buf),0);
		return;
	}
	else{
	    sprintf(buf,"[%s]:%s\n",member[index].name,msg->data);
	    while(tag){
		    member[index].auth=1;
		    sprintf(buf1,"member: %s start public_chat,group owner:[%s]!\n",member[index].name,member[index].name);
			/*
			for(i=0;i<MAX_MEMBER_NUM;i++){
				if(member[i].mark==-1){
					continue;
				}
				send(member[index].mark,buf1,strlen(buf1),0);
			}
			*/
		    tag--;
	    }
	}
	for(i=0;i<MAX_MEMBER_NUM;i++){
		if(member[i].mark==-1){
			continue;
		}
		send(member[i].mark,buf,sizeof(buf),0);
		send(member[i].mark,buf1,sizeof(buf1),0);
	}
}
//私聊
void private_chat(int index,struct protocol *msg){
	int dest_index;
	char buf[128]={0};
	dest_index=find_member(msg->name);
	if(member[index].ban==1){
		sprintf(buf,"member: %s can't chat!\n",member[index].name);
        send(member[index].mark,buf,sizeof(buf),0);
		return; 
	}
	if(dest_index==-1){
		sprintf(buf,"there is no member!\n");
		send(member[index].mark,buf,sizeof(buf),0);
		return;
	}else{
		sprintf(buf,"[%s] to [%s] :%s\n",member[index].name,member[dest_index].name,msg->data);
		send(member[dest_index].mark,buf,sizeof(buf),0);
		return;
	}
}
//修改密码
void modify_pwd(int sockfd,int *index,struct protocol *msg){
	int dest_index;
	struct protocol msg_back;
	msg_back.cmd=MODIFYPWD;
	dest_index=find_member(msg->name);
	if(dest_index==-1){
		msg_back.state=MEMURE;
		send(sockfd,&msg_back,sizeof(msg_back),0);
		printf("member: %s modify password fail!\n",msg->name);
		return;
	}else{
		strcpy(member[dest_index].passwd,msg->data);
		msg_back.state=MODIFYPWD_OK;
		send(sockfd,&msg_back,sizeof(msg_back),0);
		printf("member: %s mofidy password success!\n",msg->name);
		return;
	}
}


