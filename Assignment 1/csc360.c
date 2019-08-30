#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


typedef struct bg_pro{ 
    pid_t pid; 
    char *command[1024];
    struct bg_pro* next; 
}bg_pro;
	
	bg_pro* root;
	int bgsize= 0;

void bglist() {
	bg_pro* tmp = root;
	do{
		int i =0;
		while(tmp->command[i] != NULL){
    			printf("%s", tmp->command[i]);
			printf(" ");
    			i++;
		}
		tmp = tmp->next;
		printf("\n");
					
	}while(tmp != NULL);

	printf("%s", "Total Background jobs: ");
	printf("%d\n", bgsize);
}

void newNode(bg_pro* node, char *arr[], pid_t pid) {
	node->pid = pid;
	int i = 0;
	while(arr[i] != NULL)
	{
    		node->command[i] = arr[i];
    		i++;
	}
	node->next = NULL;
}

void partThree(char *arr[], int size) {
	pid_t pid = fork();

	if(pid < 0) {
		printf("ERROR when forking a child process\n");
		exit(0);
	}else if(pid == 0) {
		if(execvp(arr[0], arr) < 0) {
			printf("ERROR when running this command, try maning the command\n");
			printf("System will still terminate that unknown command!!!!\n");
			bgsize--;
			exit(0);
		}
	}else{
		if(bgsize == 0) {
			root = (bg_pro*)malloc(sizeof(bg_pro));
			newNode(root, arr, pid);
			bgsize++;
		}else{
			bg_pro* tmp = root;
			while(tmp->next != NULL) {
				tmp = tmp->next;			
			}
			bg_pro* new = (bg_pro*)malloc(sizeof(bg_pro));
			newNode(new, arr, pid);
			tmp->next = new;
			bgsize++;
		}
	}
}	

void partTwo(char *arr[]){
	if(arr[1] == NULL || strcmp(arr[1],"~") == 0 ) {
		if(chdir(getenv("HOME")) != 0) {
			printf("ERROR cding\n");
		}
	}else if(strcmp(arr[1],"..") == 0){
		if(chdir("..")!= 0) {
			printf("ERROR cding\n");
		}
	}else{
		if(chdir(arr[1])!= 0) {
			printf("ERROR cding, maybe this path does not exist\n");
		}			
	}
} 


int main(int argc, char *argv[]) {

do{

ssize_t lineSize;
char currd[1024];
char *input = NULL;
char *token;
char *argu[1024];

	getcwd(currd, sizeof(currd));


printf("%s", "SSI: ");
printf("%s", currd);
printf("%s", " >");

	if(getline(&input, &lineSize, stdin) == 1) {
		continue;
	}

	token = strtok(input, " \n");

	if(token == NULL) {
		continue;
	}

	argu[0] = token;

	int i =0;
	while(argu[i]!= NULL) {
		token = strtok(NULL, " \n");
		argu[i+1] = token;
		i++;
		
		if(i >= 1024) {
			argu[1024] = realloc(argu, 1024);		
		}
		
	}

	if(strcmp(argu[0], "exit") == 0) {
		printf("exit was entered!! closing now. Bye Bye~\n");
		exit(0);
	}
	else if(strcmp(argu[0], "cd") == 0) {  //go to part 2
		partTwo(argu);
	}
	else if(strcmp(argu[0], "bg") == 0) { 	//go to part 3

		int d;
		for(d=1; d<sizeof(argu); d++){
			if(argu[d] == NULL) {
				argu[d-1] = NULL;	
				break;	
			}else{
				argu[d-1] = argu[d];
			} 
		}

		partThree(argu, bgsize);
		
	
	}else if(strcmp(argu[0], "bglist") == 0) {
		if(bgsize == 0) {
			printf("%s\n", "There is no background process running right now");
		}else{
			bglist();
		}
	}
	else{
		pid_t pid = fork();
		if(pid < 0) {
			printf("ERROR when forking a child process\n");
			exit(0);
		}else if(pid == 0) {
			if(execvp(argu[0], argu) < 0) {
				printf("ERROR when running this command, try maning the command\n");
				exit(0);
			}	
		}else{
			pid_t waitpid = wait();
		}
	}
	
	if(bgsize > 0) {
		pid_t ter = waitpid(0, NULL, WNOHANG);
		while(ter > 0) {
			if(ter >0) {
				if(root->pid == ter) {
					printf("%s", "PID:");
					printf("%d", root->pid);
					printf("%s", "  ");		
					int i =0;
					while(root->command[i] != NULL){
    						printf("%s", root->command[i]);
						printf(" ");
    						i++;
					}
					printf("%s\n", "has terminated");
					root= root->next;
					bgsize--;	
				}else{
					bg_pro* tmp = root;
					while(tmp->next != NULL) {
						if(tmp->next->pid == ter) {
							printf("%s", "PID:");
							printf("%d", tmp->next->pid);
							printf("%s", "  ");
							int i=0;
							while(tmp->next->command[i] != NULL){
    								printf("%s", tmp->next->command[i]);
								printf(" ");
    								i++;
							}
							printf("%s\n", "has terminated");
							tmp->next = tmp->next->next;
							bgsize--;
						}	
					}				
				}
			}
			ter = waitpid(0, NULL, WNOHANG);			
		}	
	}
}while(1);

}






