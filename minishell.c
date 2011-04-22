#include <stdio.h>
#include <unistd.h>
char * get_prompt(){
	char *buf = (char *)malloc(sizeof(char)*1024);
	printf("%s $ ",get_current_dir_name());
	gets(buf);
	return buf;
}
int main(){
	char * cmd;
	while(1){
		cmd = get_prompt();
		free(cmd);
	}
	return 0;
}

