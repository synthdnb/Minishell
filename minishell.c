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

void trim(char *buf){
	char *x;
	int i,len;
	x = buf;
	while(*x == ' ' || *x == '\t' || *x == '\n')
		x++;
	len = strlen(x);
	for(i=0;i<len;i++)
		buf[i]=*(x+i);
	i--;
	while(i>=0 && buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')
		i--;
	buf[i+1]=0;
}
char *get_prompt(){
	char *buf = (char *)malloc(sizeof(char)*1024);
	printf("%s $ ",get_current_dir_name());
	fgets(buf,1024,stdin);
	trim(buf);
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
		while(*ed != '\n' && *ed != '\t' && *ed != 0 && *ed != '<' && *ed != '>' && *ed != ' ' && *ed != '|' )
			ed++;
		len2 = ed-p-1;

		st = p;
		while( st != cmd && *st != '\n' && *st != '\t' && *st != 0 && *st != '<' && *st != '>' && *st != ' ' && *st != '|' )
			st--;
		len1 = p-st-1;
		if( st == cmd ) len1++;
		else st++;

		dp = opendir(get_current_dir_name());

		if(dp != NULL){
			while(1){
				item = readdir(dp);
				if(item == NULL) break;
				if(*st != '.' && item->d_name[0] == '.') continue;
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
					strcpy(ptr->cmd+(st-cmd)+strlen(item->d_name),ed);
					ptr->next = NULL;
				}
			}
		}

	}else if(strchr(cmd,'*') != NULL){
		int len1,len2;
		char *st,*ed,*p;

		p = strchr(cmd,'*');

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

		if(dp != NULL){
			while(1){
				item = readdir(dp);
				if(item == NULL) break;
				if(*st != '.' && item->d_name[0] == '.') continue;
				if(ed-st-1 <= strlen(item->d_name) && strmatch(st,item->d_name,len1) && strmatch(p+1,item->d_name+strlen(item->d_name)-len2,len2)){
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
					strcpy(ptr->cmd+(st-cmd)+strlen(item->d_name),ed);
					ptr->next = NULL;
				}
			}
		}

	}else{
		ret = (exec_list *)malloc(sizeof(exec_list));
		ret->cmd = (char *)malloc(sizeof(char)*(strlen(cmd)+1));
		strcpy(ret->cmd,cmd);
		ret->next = NULL;
	}
	return ret;
}


typedef struct proc_list{
	char **argv;
	char *redir;
	int redir_type;
	int run_background;
	struct proc_list *next;
} proc_list;

char **str2argv(char *cmd){
	int i,cnt;
	int argc;
	char **argv,**ptr;
	trim(cmd);
	for(i=0,cnt=0,argc=0;i<strlen(cmd);i++){
		if(*(cmd+i) == ' ' || *(cmd+i) == '\t' || *(cmd+i) == '\n' || *(cmd+i) == 0){
			if(cnt>0){
				argc++;
				cnt=0;
			}
		}else{
			cnt++;
		}
	}
	if(cnt>0) argc++;
	argv = (char **)malloc(sizeof(char *)*(argc+1));
	for(i=0,cnt=0,ptr=argv;i<strlen(cmd);i++){
		if(*(cmd+i) == ' ' || *(cmd+i) == '\t' || *(cmd+i) == '\n' || *(cmd+i) == 0){
			if(cnt>0){
				*ptr = (char *)malloc(sizeof(char *)*(cnt+1));
				strncpy(*ptr,cmd+i-cnt,cnt);
				*((*ptr)+cnt) = 0;
				ptr++;
				cnt=0;
			}
		}else{
			cnt++;
		}
	}
	if(cnt>0){
		*ptr = (char *)malloc(sizeof(char *)*(cnt+1));
		strncpy(*ptr,cmd+i-cnt,cnt);
		*(*(ptr)+cnt) = 0;
		ptr++;
	}
	*ptr = 0;
	return argv;
}

proc_list *parse_proc(char *cmd){
	char *str = cmd,*p,*pt,*st,*ed,*ptrcmd;
	proc_list *ret,*ptr;
	ret = NULL;
	while(strchr(str,'|') != NULL){
		if(ret == NULL){
			ret = (proc_list *)malloc(sizeof(proc_list));
			ptr = ret;
		}else{
			ptr->next = (proc_list *)malloc(sizeof(proc_list));
			ptr = ptr->next;
		}
		p = strchr(str,'|');

		ptrcmd = (char *)malloc(sizeof(char)*(p-str+1));
		strncpy(ptrcmd,str,p-str);
		*(ptrcmd+(p-str)) = 0;
		trim(ptrcmd);
		if(ptrcmd[strlen(ptrcmd)-1] == '&'){
			ptrcmd[strlen(ptrcmd)-1] = 0;
			ptr->run_background = 1;
		}else{
			ptr->run_background = 0;
		}
		if((pt=strstr(ptrcmd,">>") != NULL)){
			ptr->redir_type = 3;
		}else if((pt=strchr(ptrcmd,'>')) != NULL){
			ptr->redir_type = 2;
		}else if((pt=strchr(ptrcmd,'<')) != NULL){
			ptr->redir_type = 1;
		}else{
			ptr->redir_type = 0;
		}
		if(ptr->redir_type != 0){
			st = pt;
			while(*st == ' ' || *st == '\t' || *st == '>' || *st == '<')
				st++;
			ed=st;
			while(!(*ed == ' ' || *ed == '\t' || *ed == '>' || *ed == '<' || *ed == '|' || *ed == 0 || *ed == '\n'))
				ed++;
			ptr->redir = (char *)malloc(sizeof(char)*(ed-st+1));
			strncpy(ptr->redir,st,ed-st);
			*(ptr->redir+(ed-st)) = 0;
			strcpy(pt,ed);
		}else{
			ptr->redir = NULL;
		}
		ptr->argv = str2argv(ptrcmd);
		str = p+1;
		ptr->next = NULL;
		free(ptrcmd);
	}
	if(ret == NULL){
		ret = (proc_list *)malloc(sizeof(proc_list));
		ptr = ret;
	}else{
		ptr->next = (proc_list *)malloc(sizeof(proc_list));
		ptr = ptr->next;
	}
	ptrcmd = (char *)malloc(sizeof(char)*(strlen(str)+1));
	strcpy(ptrcmd,str);
	trim(ptrcmd);
	if((pt=strstr(ptrcmd,">>") != NULL)){
		ptr->redir_type = 3;
	}else if((pt=strchr(ptrcmd,'>')) != NULL){
		ptr->redir_type = 2;
	}else if((pt=strchr(ptrcmd,'<')) != NULL){
		ptr->redir_type = 1;
	}else{
		ptr->redir_type = 0;
	}
	if(ptr->redir_type != 0){
		st = pt;
		while(*st == ' ' || *st == '\t' || *st == '>' || *st == '<')
			st++;
		ed=st;
		while(!(*ed == ' ' || *ed == '\t' || *ed == '>' || *ed == '<' || *ed == '|' || *ed == 0 || *ed == '\n'))
			ed++;
		ptr->redir = (char *)malloc(sizeof(char)*(ed-st+1));
		strncpy(ptr->redir,st,ed-st);
		*(ptr->redir+(ed-st)) = 0;
		strcpy(pt,ed);
	}else{
		ptr->redir = NULL;
	}

	ptr->argv = str2argv(ptrcmd);
	ptr->next = NULL;
	free(ptrcmd);
	return ret;
}

void print_proc(proc_list *x){
	if(!x) return;
	int i;
	for(i=0;x->argv[i];i++){
		printf("argv[%d] : %s\n",i,x->argv[i]);
		free(x->argv[i]);
	}
	free(x->argv);
	if(x->redir_type){
		printf("Redir Type :%d\n",x->redir_type);
		printf("Redir File :%s\n",x->redir);
		free(x->redir);
	}
	if(x->next){ printf("== Next Proc ==\n"); print_proc(x->next);}
	else{printf("==Proc End==\n");}
	free(x);
}

void exec_proc(proc_list *proclist){
	int pip;
	int ifile,ofile;
	while(proclist){
		if(proclist->next){
		}
	}
}
void execute(char *cmd){
	printf("EXECUTE CMD: %s\n",cmd);
	proc_list *proclist = parse_proc(cmd);
	exec_proc(proclist);
	//print_proc(proclist);
}

int main(){
	char *cmd;
	exec_list *execlist,*ptr;
	while(1){
		cmd = get_prompt();
		if(strcmp(cmd,"exit") == 0){free(cmd); break;}
		execlist = parse_wildcards(cmd);
		free(cmd);
		if(execlist == NULL) fprintf(stderr,"No Matching Wildcards\n");
		else{
			while(execlist != NULL){
				execute(execlist->cmd);
				free(execlist->cmd);
				ptr = execlist->next;
				free(execlist);
				execlist = ptr;
			}
		}
		
	}
	return 0;
}

