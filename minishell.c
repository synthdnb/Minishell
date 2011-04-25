#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <malloc.h>
typedef struct command{
	char *proc_name;
	char **argv;
} command;

char * get_prompt(){
	char *buf = (char *)malloc(sizeof(char)*1024);
	printf("%s $ ",get_current_dir_name());
	fgets(buf,1024,stdin);
	return buf;
}

int strmatch(char *a,char *b,int n){
	int i;
	for(i=0;i<n;i++)
		if(*(a+i) != *(b+i)) return 0;
	return 1;
}

typedef struct exec_list{
	char *cmd;
	struct exec_list *next;
} exec_list;

exec_list *parse_wildcards(char *cmd){
	exec_list *ret,*ptr;
	DIR *dp;
	struct dirent *item;
	ret = NULL;
	if(strchr(cmd,'?')){
		int len1,len2;
		char *st,*ed,*p;

		p = strchr(cmd,'?');

		ed = p;
		while(*ed != '\n' && *ed != 0 && *ed != '<' && *ed != '>' && *ed != ' ' && *ed != '|' )
			ed++;
		len2 = ed-p-1;

		st = p;
		while( st != cmd && *st != '\n' && *st != 0 && *st != '<' && *st != '>' && *st != ' ' && *st != '|' )
			st--;
		len1 = p-st-1;
		if( st == cmd ) len1++;
		else st++;

		dp = opendir(get_current_dir_name());

		printf("%d %d\n",len1,len2);
		if(dp != NULL){
			while(1){
				item = readdir(dp);
				if(item == NULL) break;
//				if(strmatch(x,item->d_name,len1) && strmatch(p+1,item->d_name+strlen(item->d_name)-len2,len2))
				if(ed-st == strlen(item->d_name) && strmatch(st,item->d_name,len1) && strmatch(p+1,item->d_name+len1+1,len2)){
					if( ret == NULL ){
						ptr = (exec_list *)malloc(sizeof(exec_list));
						ret = ptr;
					}else{
						ptr->next = (exec_list *)malloc(sizeof(exec_list));
						ptr = ptr->next;
					}
					int newlen = strlen(cmd)-(ed-st)+strlen(item->d_name)+1;
					ptr->cmd = (char *)malloc(sizeof(char)*newlen);
					strncpy(ptr->cmd,cmd,st-cmd);
					strncpy(ptr->cmd+(st-cmd),item->d_name,strlen(item->d_name));
					strncpy(ptr->cmd+(st-cmd)+strlen(item->d_name),ed,strlen(ed));
					printf("CMD : %s",ptr->cmd);
					ptr->next = NULL;
				}
			}
		}

	}else if(strchr(cmd,'*') != NULL){
		char *x = strchr(cmd,'*');
		while( x != cmd && *x != '<' && *x != '>' && *x != ' ' && *x != '|' )
			x--;
		dp = opendir(get_current_dir_name());
		if(dp != NULL){
			while(1){
				item = readdir(dp);
				if(item == NULL) break;
//				strchr(
				printf("DIR : %s\n",item->d_name);
			}
		}
	}else{
		ret = (exec_list *)malloc(sizeof(exec_list));
		ret->cmd = (char *)malloc(sizeof(char)*strlen(cmd));
		strcpy(ret->cmd,cmd);
		ret->next = NULL;
	}
	return ret;
}
int main(){
	char * cmd;
	char ** cmdlist;
	while(1){
		cmd = get_prompt();
		cmdlist = parse_wildcards(cmd);
		free(cmd);
		
	}
	return 0;
}

