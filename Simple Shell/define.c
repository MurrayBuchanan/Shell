
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>


// Max alias
#define MAX_alias 10

// Struct to store aliases
typedef struct Aliases {
    char alias_name[10];
    char prevCommandName[10];
} Alias;

// Struct to store aliases and count
typedef struct {
    Alias aliases[MAX_alias];
    int count;
} AliasTable;
AliasTable aliasTable;

typedef struct history{
	char *command[128];
	struct history *next;
} history;
typedef history** List;

List new_list(void);
history *new_history(char *element[128]);
char currentInputArray[21][512]; // for processing loading history
bool processedFlag = false;
int histCounter = 0;
int histCounter2 = 0;
char dir[1024];

void display_prompt(void);
void homeDirectory(void);
void exitShell(void);
void childProcess(char *thecommand[128], int numTerms);
void processHistoryPeek(List head_ref);
void tokenize(char currentInput[512], char *inputLine[128], int numTerms, int currentTerm);

void setpath(char* path);
void getpath(void);
void setOriginalPath(void);
void restoreOriginalPath(void);
void changeDirectory(char *inputLine[]);
void printWorkingDirectory();
void homeDirectory(void);
void addAlias(char *alias_name, char *prevCommandName);
void removeAlias(char *alias_name);
void printAliases(void);
void checkAlias(char *inputLine[128]);

void push(List history_head, char *element[128]);
void commandList(char *element[128]);
void processHistory(char *thecommand[128]);
void print_list(List head_ref);
void removeFirst(void);
int sizeOfList(List head_ref);
List historyArray;
void loadHistory(void);
void loadAliases(void);

void addAlias(char *alias_name, char *prevCommandName);
void removeAlias(char *alias_name);
void printAliases(void);
void checkAlias(char *inputLine[128]);
void clear_history(List head_ref);
void delete_list(List list);

char originalPath[1024];

// Function the user input
void display_prompt(void) {
	printWorkingDirectory();

    char currentInput[512];
	if (histCounter2 == histCounter-1 || processedFlag) {
		printf("%s %% ", dir);
	} else {
		strcpy(dir, "copying history");
		printf("%s %d %%\n",dir, histCounter2 );
	}
		
	if (!processedFlag && currentInputArray[histCounter2] != NULL) {
		strcpy(currentInput, currentInputArray[histCounter2]);
		if (histCounter2 >= histCounter-1) {
			processedFlag = true;
		}
		histCounter2++;
	} else {
		processedFlag = true;
	}
	if (processedFlag) {
		if (fgets(currentInput, 512, stdin) == NULL) {
			char* temp[128];
			temp[0] = "exit";
			push(historyArray, temp);
			exitShell();
		}
	}
	
    if (strcmp(currentInput, "\n") == 0) {
        return;
    }

    // Tokenize input
    char *inputLine[128];
	int numTerms = 0;
    int currentTerm = 0;

    char *token = strtok(currentInput, " \n\t;&><|");
    while (token != NULL) {
        inputLine[currentTerm] = token;
        token = strtok(NULL, " \n\t;&><|");
        currentTerm++;
        numTerms++;
    }
    if (currentTerm > 0) {
        inputLine[currentTerm] = NULL; 
    } 

    // If no al1ias found, proceed with regular parsing
    if (inputLine[0][0] != '!' && strcmp(inputLine[0], "\n") != 0 && strcmp(inputLine[0], "clearHistory") != 0) {
        push(historyArray, inputLine);
    }

	// Check for alias and replace if needed
    checkAlias(inputLine);
	
	if (processedFlag) {
		childProcess(inputLine, numTerms);
		commandList(inputLine);
	}
}

void commandList(char *element[128]){
	if (strcmp(element[0], "exit") == 0) {
		exitShell();
		return;
	} else if (strcmp(element[0], "cd") == 0) {
		changeDirectory(element);
		return;
	}  else if (strcmp(element[0], "pwd") == 0) {
        printWorkingDirectory();
        return;
    } else if (strcmp(element[0], "ls") == 0) {
        return;
	} else if (strcmp(element[0], "clear") == 0) {
        return;
	} else if (strcmp(element[0], "\n") == 0) {
		return;
	} else if (strcmp(element[0], "ps") == 0) {
		return;
	} else if (strcmp(element[0], "getpath") == 0) {
		if (element[1] != NULL) { 
			printf("...too many arguments provided \n");
        } else {
            getpath();
			printf("\n");
        }
		return;
    } else if (strcmp(element[0], "setpath") == 0) {
		if (element[1] == NULL) {
			printf("...Not enough arguments provided \n");
		}
		else if (element[2] != NULL) {
			printf("...too many arguments provided \n");
		}
		else {
			setpath(element[1]);
		}
        
		return;
	} else if(strcmp(element[0], "!!") == 0) {
		if (element[1] != NULL) {
			printf("...too many arguments provided \n");
		} else {
			processHistoryPeek(historyArray);
		}
		return;
	} else if (element[0][0] == '!') {
		processHistory(element);
		return;
	} else if(strcmp(element[0], "history") == 0) {
		if (element[1] != NULL) {
			printf("...too many arguments provided \n");
		} else {
			print_list(historyArray);
		}
		return;
	} else if(strcmp(element[0], "alias") == 0) {
		if(element[1] != NULL && element[2] == NULL){
			printf("...Not enough arguments provided \n");
		}
		else if (element[1] != NULL && element[2] != NULL) { 
			addAlias(element[1], element[2]);
			return;
		} else {
			printAliases();
			return;
		}
	} else if(strcmp(element[0], "unalias") == 0) {
		removeAlias(element[1]);
		return;
	} else if (strcmp(element[0], "clearHistory") == 0) {
		clear_history(historyArray);
		return;
	}else {
        printf("Command not found: %s\n", element[0]);
        return;
    }
}

void childProcess(char *thecommand[128], int numTerms) {
	pid_t pid = fork();

    if (pid < 0) {
        perror("...fork failed");
    } else if (pid == 0) {
        thecommand[numTerms] = NULL;
		execvp(thecommand[0], thecommand);
		exit(1); // deletes child process at end of runtime
    } else {
        wait(NULL);
    }
}
void tokenize(char currentInput[512], char *inputLine[128], int numTerms, int currentTerm){
    char *token = strtok(currentInput, " \n\t;&><|");
    while (token != NULL) {
        inputLine[currentTerm] = token;
        token = strtok(NULL, " \n\t;&><|");
        currentTerm++;
        numTerms++;
    }
    if (currentTerm > 0) {
        inputLine[currentTerm] = NULL; // null 
    } 

    // Check for alias and replace if needed
    checkAlias(inputLine);

}

void setpath(char* path) {
    if (setenv("PATH", path, 1) == 0) {
		printf("%s", path);
    } else {
        printf("...failed to set path\n");
    }
}

void getpath(void) {
	char* path = getenv("PATH");
    if( path != NULL) {
        printf("%s", path);
    } else {
        perror("...cannot find the path\n");
    }
}

void setOriginalPath(void) {
    char* path = getenv("PATH");
    if (path != NULL) {
        strncpy(originalPath, path, sizeof(originalPath));
        originalPath[sizeof(originalPath) - 1] = '\0';
    } else {
        printf("...cannot find the path\n");
    }
}

void restoreOriginalPath(void) {
    if (setenv("PATH", originalPath, 1) == 0) {
		printf("%s\n", originalPath);
		printf("...restored and saved path\n");
    } else {
        perror("...cannot restore the path\n");
    }
} 

void changeDirectory(char *inputLine[]) {
    if (inputLine[1] == NULL) {
        printf("No argument provided \n");
        homeDirectory(); 
    } else if (inputLine[2] != NULL) {
        printf("Too many arguments provided \n");
    } else {
        if (chdir(inputLine[1]) != 0) { 
            printf("No such file or directory \n");
        }
    }
}

void printWorkingDirectory() {
    if (getcwd(dir, sizeof(dir)) == NULL) {
        printf("getcwd() error");
        return ;
    }
}

void homeDirectory(void) {
	char* home = getenv("HOME");
    if (home != NULL) {
		chdir(home);
    } else {
        perror("...error finding home directory\n");
    }
}

void addAlias(char *alias_name, char *prevCommandName) {
	if (aliasTable.count < MAX_alias){
		for(int i = 0; i < aliasTable.count;i++ ){
			if(strcmp(aliasTable.aliases[i].alias_name, alias_name) == 0){
				printf("...Alias name already in use \n");
				return;
			}	
		}
		// Copys the alias and the command into the structs
		strcpy(aliasTable.aliases[aliasTable.count].alias_name, alias_name);
		strcpy(aliasTable.aliases[aliasTable.count].prevCommandName, prevCommandName);
		printf("Alias added successfully!\n");
	aliasTable.count++;
	} else {
		printf("... error max Number of Aliases reached.\n");
	}
}

void removeAlias(char *alias_name) {
	int i = 0;
	int found = 0;
	
    for (i = 0; i < aliasTable.count; i++) {
        if (strcmp(aliasTable.aliases[i].alias_name, alias_name) == 0) {
            found = 1;
            break;
        }
    }
    if (found) {
        for (; i < aliasTable.count - 1; i++) {
            aliasTable.aliases[i] = aliasTable.aliases[i + 1];
        }
        aliasTable.count--;
		printf("Alias removed.\n");
    } else {
        printf("...error alias not found.\n");
    }
}

void printAliases(void) {
    if (aliasTable.count == 0) {
        printf("No aliases stored...\n");
        return;
    }

    printf("Aliases:\n");
    for (int i = 0; i < aliasTable.count; i++) {
        printf("%s = %s\n", aliasTable.aliases[i].alias_name, aliasTable.aliases[i].prevCommandName);
    }
}

void checkAlias(char *inputLine[128]) {
    for (int i = 0; i < aliasTable.count; i++) {
        if (strcmp(inputLine[0], aliasTable.aliases[i].alias_name) == 0) {
            // Replace alias with aliased command
            strcpy(inputLine[0], aliasTable.aliases[i].prevCommandName);
            
            // Concatenate
            for (int j = 1; j < 128 && inputLine[j] != NULL; j++) {
                strcat(inputLine[0], " ");
                strcat(inputLine[0], inputLine[j]);
            }
            return;
        }
    }
}	
	
List new_list(void){
	
	List list = malloc(sizeof(List));
	*list = NULL;
	return list;
}

history *new_history(char *element[128]){
	history *new = (history*)malloc(sizeof(history));

	
	for(int i = 0; element[i] != NULL && i < 128; i++){
		new->command[i] = (char*)malloc(sizeof(element[i]));
		//strcpy(new->command[i], element[i]);
		new->command[i] = strdup(element[i]);
    }
	
	new->next = NULL;
	return new;
}

void push(List history_head, char *element[128]) {
	history *temp = *history_head; 
	history *additional = new_history(element);
	
	if (temp == NULL) {
        *history_head = additional;
        return;
    }
	int counter = 0;
	while (temp->next != NULL) {
		counter++;
		temp = temp->next;
	}
	temp->next = additional;
	if (sizeOfList(historyArray) > 20) { // ensures list does not exceed 20
		removeFirst(); 
	}	
}

void removeFirst(void) {
	if (*historyArray == NULL) {
		return;
	}
	history *temp = *historyArray; 
	*historyArray = (*historyArray)->next; // assuming head_ref is not null as only called when list exceeds 20
	
	temp = NULL;
	free(temp);
}

void processHistory(char *thecommand[128]){
	int index;
	checkAlias(thecommand);
	if (thecommand[1] != NULL) {
		index = atoi(thecommand[1])-1; // converts second token 
	} else if (thecommand[0][0] != '\0') {
		char *pointer = thecommand[0];
		pointer++;
		index = atoi(pointer)-1;
	}

	if ((index < 0) || index >= sizeOfList(historyArray)){
		printf("...Invalid input\n");
		return; // returns 0 when non-numeric attributes appear in the command, or when the user inputs 0 which is an invalid command is it starts from 1
	}

	if (*historyArray == NULL) {
		printf("No commands in history\n");
	}

	history *temp = *historyArray;
	int counter = 0;

	while( temp != NULL && counter < index) {
		counter ++;
		temp = temp->next;
	}

	int numTerms = 0;
	while (temp->command[numTerms] != NULL) { 
		numTerms++;
	}

	if (temp == NULL) {
		return;
	} else {
		childProcess(temp->command, numTerms);
		commandList(temp->command);
		return;
	}
}

void processHistoryPeek(List head_ref) {
	if (*head_ref == NULL) {
		return;
	}
	history *temp = *head_ref; 
	while(temp->next != NULL) {
		temp = temp->next;
	}
	int numTerms = 0;
	while (temp->command[numTerms] != NULL) {
		numTerms++;
	}
	childProcess(temp->command, numTerms);
	commandList(temp->command);
}

void print_list(List head_ref) {
	
	history *temp = *head_ref;
	if (*head_ref == NULL) {
		printf("\n");
		return;
	}

	int counter = 0;
	while (temp != NULL) {
		counter++; //counter for elements in historyArray	
		printf("%d ",counter);
		for (int i = 0; i < 127 ; i++) {
			if (temp->command[i] != NULL) {
				printf("%s ", temp->command[i]); // {counter} {element 1}...{element n}
			} else {
				break;
			}
		}
		
		printf("\n"); // {counter} {element 1}...{element n} \n
		if (temp->next == NULL) {
			break;
		}
		temp = temp->next;
	}
}

int sizeOfList(List head_ref) {
	int counter =  1;
	history *temp = *head_ref;

	if (*head_ref == NULL) {
		return 0;
	}
	while (temp->next != NULL) {
		counter ++;
		temp = temp->next;
	}
	return counter;
}

void saveHistory(void) {
    homeDirectory();
    FILE *file = fopen(".hist_list", "w");
    if (file == NULL){
        perror("Error opening .hist_list");
	    fclose(file);
        return;
    }

    history *temp = *historyArray; // First element
    while (temp != NULL) {
	    int counter = 0;
	    while (temp->command[counter] != NULL) {
		    fprintf(file, "%s ",temp->command[counter]);
		    counter++;
	    }
	    fprintf(file, "\n");
	    temp = temp->next;
    }
    fclose(file);
    printf("...saved history\n");
}

void loadHistory(void){
	FILE *file = fopen(".hist_list", "r");
	if (file == NULL) {
		FILE *file = fopen(".hist_list", "w");
		fclose(file);
		loadHistory();
		return;
	}
	char currentInput1[512];
	histCounter = 0;
	
	if (file != NULL) {
		while (histCounter < 21 && fgets(currentInput1, sizeof(currentInput1), file)) {
			strcpy(currentInputArray[histCounter], currentInput1);
			histCounter++;
		}
	}
	fclose(file);
}

void clear_history(List head_ref){
	if (*head_ref == NULL) {
		printf("...no history stored \n");
		return;
	}
	while (*head_ref != NULL) {
        history *temp = (*head_ref)->next;
		free(*head_ref);
		*head_ref = NULL;
        *head_ref = temp;
    }
	printf("...cleared history \n");
}

void delete_list(List list){
	while (*list != NULL) {
        history *temp = (*list)->next;
		free(*list);
		*list = NULL;
        *list = temp;
    }
	free(*list);
	*list = NULL;
    free(list);
}

void saveAliases(void){
    homeDirectory();
    FILE *file = fopen(".aliases", "w");
    if (file == NULL) {
        perror("Error opening .aliases");
	    fclose(file);
        return;
    }
    for(int i = 0; i <aliasTable.count; i++){
	    fprintf(file, "%s %s\n", aliasTable.aliases[i].alias_name, aliasTable.aliases[i].prevCommandName);
    }
    fprintf(file, "\n");
    fclose(file);
    printf("...saved Aliases\n");
}

void loadAliases(void){
	FILE *file = fopen(".aliases", "r");
	if(file == NULL){
		FILE *file = fopen(".aliases", "w");
		fclose(file);
		loadAliases();
		return;
	}

	char alias_name[10];
	char prevCommandName[10];

	aliasTable.count = 0;

	while (fscanf(file, "%s %s", alias_name, prevCommandName) == 2 && aliasTable.count < MAX_alias) {
		strcpy(aliasTable.aliases[aliasTable.count].alias_name, alias_name);
		strcpy(aliasTable.aliases[aliasTable.count].prevCommandName, prevCommandName);
		aliasTable.count++;
	}
	fclose(file);
}

void exitShell(void) {
	printf("\nSaving session...\n");
	restoreOriginalPath();
	saveHistory(); //called after restoring originalPath
	saveAliases();
	
	clear_history(historyArray);
	delete_list(historyArray);
	
	printf("...completed.\n");
    exit(0);
}
