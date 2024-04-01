#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "define.c"

int main(void) {
	homeDirectory();
	setOriginalPath();
	
	historyArray = new_list();
	loadHistory();
	loadAliases();

	while(true) {
        display_prompt();
    }
    return 0;
}