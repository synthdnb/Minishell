#include <stdio.h>
#include <unistd.h>
int main(){
	char * buf = get_current_dir_name();
	printf("%s",buf);
	return 0;
}
