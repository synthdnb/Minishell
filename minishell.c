#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct command{
	char *proc_name;
	char **argv;
} command;

char * get_prompt(){
	char *buf = (char *)malloc(sizeof(char)*1024);
	printf("%s $ ",get_current_dir_name());
	gets(buf);
	return buf;
}

char ** parse_wildcards(char * cmd){
	char ** ret;
	if(strchr(cmd,'?')){
	}else if(strchr(cmd,'*'){
	}else{
		ret = (char**)malloc(sizeof(char*));
		strcpy(*ret,cmd);
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

