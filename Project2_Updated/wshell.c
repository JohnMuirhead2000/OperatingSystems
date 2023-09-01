#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdio.h>
#include<string.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_BUFFER 200
#define MAX_ARGUMENTS 100

// main commands
void exit_command();
void echo_command(char* arguments[MAX_ARGUMENTS]);
void cd_command(char* arguments[MAX_ARGUMENTS]);
void pwd_command();
void add_to_commands(char* line);
void history_command();

// helper functions
void build_string_array(char* words[MAX_ARGUMENTS], char* next_words[MAX_ARGUMENTS], char line[], int* clear_mode, int* next_amp);
int count_spaces(char string[]);
char* get_directory();
int get_array_size(char* arguments[MAX_ARGUMENTS]);
void itoa(int val);
void fix_process_string(char* process_stirng, char fixed_string[MAX_BUFFER]);
void run_as_main_process(char* arguments[MAX_ARGUMENTS], char* next_argument[MAX_ARGUMENTS], int* clear_mode, int* next_amp, char* running_in_background[255], int running_process_ID[255]);
void run_as_background_process(char* arguments[MAX_ARGUMENTS], int* task_number, int running_process_ID[255], char* running_in_background[255]);
void check_finished_processes(int running_process_ID[255], char* running_in_background[255]);
void jobs_command(char* running_in_background[255]);
void kill_command(char* arguments[MAX_ARGUMENTS], char* running_in_background[255], int running_process_ID[255]);


// necissary globals to make the function work 
char* recent_commands[10];
char numString[3];


int main(int argc, char *argv[]) 
{
    char* running_in_background[255];
    for(int i = 0; i < 10; i++) {
        recent_commands[i] = malloc(sizeof(char)*MAX_BUFFER);
    }

    for(int i = 0; i < 255; i++) {
        running_in_background[i] = malloc(sizeof(char)*MAX_BUFFER);
    }

    int running_process_ID[255] = {0};


    char* next_argument[MAX_ARGUMENTS];
    next_argument[0] = "";

    // when we get the ||; we are in 'clear mode'
    int clear_mode = 0;
    int next_amp = 0;

    //stores what the current task number is 
    int task_number = 1;

    //it must enter an infiniate while loop and wait for command line input: 
    while(1)
    { 
      
        char* arguments[MAX_ARGUMENTS];


        // first it checks if any of the running process are finished
        check_finished_processes(running_process_ID, running_in_background);

        // checks if it has another command in queue (form && or ||) and if it doesent, gets standard input
        if (strcmp(next_argument[0], "") == 0)
        {
            char* currentDirecotry = get_directory();
            printf("%s$ ", currentDirecotry);
            free (currentDirecotry);
            char line[MAX_BUFFER];
            fgets(line, MAX_BUFFER, stdin);
            add_to_commands(line);
            if (! isatty(fileno(stdin))) { 
                printf("%s", line);
                fflush(stdout); 
            }
            build_string_array(arguments, next_argument, line, &clear_mode, &next_amp);  
        } else {
            memcpy(arguments, next_argument, sizeof(next_argument));
            next_argument[0] = "";
        }
        
        // if the command it indicated as backgorund (with &) it runs it as a beackgorund task. Otherwise, it runs as a main task. 
        int size = get_array_size(arguments);
        if (strcmp(arguments[size-1], "&") == 0){
            strcpy(arguments[size-1], "");
            run_as_background_process(arguments, &task_number, running_process_ID, running_in_background);
        } else {
            run_as_main_process(arguments, next_argument, &clear_mode, &next_amp, running_in_background, running_process_ID);
        }
    }
}


// polls all the processes and gives a print message for those that are finished 
void check_finished_processes(int running_process_ID[255], char* running_in_background[255]){
    for (int i = 1; i < 255; i++){
        int current_id = running_process_ID[i];
        if (current_id != 0){
            int status;
            int result = waitpid(current_id, &status, WNOHANG);
            if (result > 0){
                // the process is complete!
                //make a print statemenmt; clear it from the running process
                char fixed_string[MAX_BUFFER];
                memset(fixed_string, 0, strlen(fixed_string));

                fix_process_string(running_in_background[i], fixed_string);

                printf("[%d] Done:%s\n", i, fixed_string);
                running_process_ID[i] = 0;
                strcpy(running_in_background[i], "");
            }
        }
    }
}

//adjusts the process stirng to fit the format we want
void fix_process_string(char* process_stirng, char fixed_string[MAX_BUFFER]){
    int relevant_index = 2;
    for (int i = relevant_index; i < strlen(process_stirng); i++){
        fixed_string[i - relevant_index] = process_stirng[i];
        fixed_string[i-relevant_index+1] = '\0';
    }
}

// runs the arguments in the main process
void run_as_main_process(char* arguments[MAX_ARGUMENTS], char* next_argument[MAX_ARGUMENTS], int* clear_mode, int* next_amp, char* running_in_background[255], int running_process_ID[255]){

    char* command = arguments[0];
    if (strcmp(command, "exit") == 0){
        exit_command();
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    } else if(strcmp(command, "echo") == 0){
        echo_command(arguments);
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    } else if(strcmp(command, "cd") == 0) { 
        cd_command(arguments);
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    } else if(strcmp(command, "pwd") == 0) {
        pwd_command();
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    } else if(strcmp(command, "history") ==0 ) {
        history_command();
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    } else if(strcmp(command, "jobs") ==0 ){
        jobs_command(running_in_background);
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    } else if (strcmp(command, "kill") ==0 ){
        kill_command(arguments, running_in_background, running_process_ID);
        if (*clear_mode){
        next_argument[0] = "";
        *clear_mode = 0;
        }
    }
    else{
        int rc = fork();
        if (rc < 0){
            printf("fork failed ):\n");
        } else if (rc == 0){
            arguments[get_array_size(arguments)] = NULL;
            if (execvp(arguments[0], arguments) == -1){
                printf("wshell: could not execute command: %s\n", command);
                exit(1);
            }
        } else {
            // we are in parocess
            // wait(NULL);
            int status;
            waitpid(rc, &status, 0);
            if (WIFEXITED(status)){
                int exit_status = WEXITSTATUS(status);
                if (!(exit_status)&&(*clear_mode)){
                    next_argument[0] = "";
                    *clear_mode = 0;
                } 
                if ((exit_status)&&(*next_amp)){
                    next_argument[0] = "";
                    *next_amp = 0;
                } 
            }
        }
    }
}

// performs the kill functioanlity
void kill_command(char* arguments[MAX_ARGUMENTS], char* running_in_background[255], int running_process_ID[255]){

    int arg1 = (int) strtol(arguments[1], (char**)NULL, 10);

    int process_pid = running_process_ID[arg1];

    if (process_pid == 0){
        printf("wshell: no such background job: %d\n", arg1);
        return;
    }

    // we kill the process
    kill(process_pid, SIGKILL);

    strcpy(running_in_background[arg1], "");
    running_process_ID[arg1] = 0;
}

//performs the jobs functioanlity 
void jobs_command(char* running_in_background[255]){
    for (int i = 0; i < 255; i++){
        if (strcmp(running_in_background[i], "") !=0){
            printf("%s\n",running_in_background[i]);
        }
    }
}

//runs the given argument as a background task
void run_as_background_process(char* arguments[MAX_ARGUMENTS], int* task_number, int running_process_ID[255], char* running_in_background[255]){
    itoa(*task_number);
    char numStirgCopy[MAX_BUFFER];
    strcpy(numStirgCopy, numString);

    strcat(numStirgCopy, ": ");
    strcat(numStirgCopy, arguments[0]);
    strcat(numStirgCopy, " ");
    strcat(numStirgCopy, arguments[1]);

    if (strcmp(arguments[2], "") != 0){
        strcat(numStirgCopy, " ");
        strcat(numStirgCopy, arguments[2]);
    }

    //print the task number we are with
    printf("[%d]\n", *task_number);
    strncpy(running_in_background[*task_number], numStirgCopy, strlen(numStirgCopy));

    int rc = fork();
    if (rc < 0){
        printf("fork failed ):\n");
    } else if (rc == 0){
        arguments[get_array_size(arguments)] = NULL;
        if (execvp(arguments[0], arguments) == -1){
            printf("wshell: could not execute command: %s\n", arguments[0]);
            exit(1);
        }
    } else {
        running_process_ID[*task_number] = rc;
        *task_number = *task_number + 1;
    }
}

// puts the given int val into a charcter array. Assumes val is 3 or less digits. 
void itoa(int val){
    for (int i = 0; i < strlen(numString); i++){
        numString[i] = '\0';
    }
    if (val < 10){
        numString[0] = val + '0';
        numString[1] = '\0';
    } else if ((val > 9) && (val < 100)){
        numString[0] = ((val - (val % 10)) / 10) + '0';
        numString[1] = (val % 10) + '0';
        numString[2] = '\0';
    }
    else {
        numString[0] = (val / 100)+ '0';
        numString[1] = (((val - (val % 10)) / 10) % 10)+ '0';
        numString[2] = (val % 10)+ '0';
        numString[3] = '\0';
    }

}

// given an input line, builds an array of strings for the program to use. 
void build_string_array(char* words[MAX_ARGUMENTS], char* next_words[MAX_ARGUMENTS], char line[], int * clear_mode, int* next_amp){

    char *delimiter = " ";
    char *p;
    int i = 0;
    int j = 0;

    int hitNewArg = 0;


    int result = isspace(line[strlen(line)-2]);

    int last_is_space;

    if (result == 0){
        last_is_space = 0;
    } else {
        last_is_space = 1;
    }

    p = strtok(line, delimiter);
    //hard code remove last element of p:
    while (p != NULL) {
        if (!hitNewArg){
            if ((strcmp(p, "&&")!=0) && (strcmp(p, "||")!=0)){
                words[i++] = p;
            } else {
                if (strcmp(p, "&&")==0){
                    *next_amp = 1;
                }
                if (strcmp(p, "||")==0){
                    *clear_mode = 1;
                }
                words[i++] = "";
                hitNewArg = 1;
            }
        } else {
            next_words[j++] = p;
        }
        p = strtok(NULL, delimiter);
    }
    
    // cut last off of first args if we never had more args:
    if (j == 0){
        if (!last_is_space){
            char* last_word = words[i-1];
            last_word[strlen(last_word)-1] = '\0';
            words[i-1] = last_word;
            words[i] = ""; 
        } else {
            char* last_word = words[i-1];
            last_word[strlen(last_word)-1] = ' ';
            last_word[strlen(last_word)] = '\0';
            words[i-1] = last_word;
            words[i] = ""; 
        }
    } else {
        if (!last_is_space){
            char* last_word_new = next_words[j-1];
            last_word_new[strlen(last_word_new)-1] = '\0';
            next_words[j-1] = last_word_new;
            next_words[j] = ""; 
        } else {
            char* last_word_new = next_words[j-1];
            last_word_new[strlen(last_word_new)-1] = ' ';
            last_word_new[strlen(last_word_new)] = '\0';
            next_words[j-1] = last_word_new;
            next_words[j] = ""; 
        }
    }
}

// returns 0 if it executed command, 1 i not
void exit_command(){
    //regardless of arguments or arg count, it should jsut do this
    exit(0);
}

// performs echo functionality 
void echo_command(char* arguments[MAX_ARGUMENTS]){
    char* final_argument = arguments[get_array_size(arguments)-1];

    int final_is_space = isspace(final_argument[0]);

    int count_amount;
    if (final_is_space){
        count_amount = get_array_size(arguments) -1;
    } else {
        count_amount = get_array_size(arguments);
    }

    for (int i = 1; i < count_amount; i++){
        if (i == count_amount-1){
            if (final_is_space){
                printf("%s ", arguments[i]);
            } else {
                printf("%s", arguments[i]);
            }
        } else{
            //space on the non last one
            printf("%s ", arguments[i]);
        }
    }
    printf("\n");
}

// performs cd functionality 
void cd_command(char* arguments[MAX_ARGUMENTS]){
    //funciton should navigate to the desired path

    int num = get_array_size(arguments);

    if (num > 2){
        printf("wshell: cd: too many arguments\n"); 
        return;   
    }
    char* path;
    if (num == 2){
        path = arguments[1];
    } else {
        path = getenv("HOME");
    }

    if (chdir(path) == 0){
        return;
    }else{
        printf("wshell: no such directory: %s\n", path);
    }
}

// performs pwd functionality 
void pwd_command(){
    char path[MAX_BUFFER];
    getcwd(path, MAX_BUFFER);
    printf("%s\n", path);
}

// function counts the number of spaces in a stirng
int count_spaces(char string[]){
    int spaces = 0;
     for(int i = 0; string[i] != '\0'; i++){
     if (string[i] == ' ')
     {
        spaces++;
     }
  }
  return spaces;
} 

//retrives the current directory 
char* get_directory(){
    char path[MAX_BUFFER];
    getcwd(path, MAX_BUFFER);

    char* return_val = malloc(strlen(path)*sizeof(char));
    if (strlen(path)==1){
        return_val[0] = '/';
        return_val[1] = '\0';
        return return_val;
    }

    int relevant_index = 0;
    for (int i = 0; i < strlen(path); i++){
        if (path[i]=='/'){
            relevant_index = i;
        }
    }

    for (int i = relevant_index+1; i < strlen(path); i++){
        return_val[i - relevant_index-1] = path[i];
    }
    return return_val;
}

// function assumes the array is "" terminated and returns the number of elemnts
int get_array_size(char* arguments[MAX_ARGUMENTS]){
    int i = 0;
    while (strcmp(arguments[i], "") != 0){
        i++;
    }
    return i;
}

//adds a given line ot the list of recent commands
void add_to_commands(char* line){
    for(int i = 9; i > 0; i--){
        strcpy(recent_commands[i], recent_commands[i-1]);
    }
    strcpy(recent_commands[0], line);
}

// performs the history functioanlity 
void history_command(){
    for(int i = 9; i > -1; i--){
        if(strcmp(recent_commands[i],"") != 0) {
            printf("%s", recent_commands[i]);
        }
    }
}