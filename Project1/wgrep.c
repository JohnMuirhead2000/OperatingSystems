#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

#define BUFFERSIZE 100
#define MAX_LINE_LENGTH 1024

void searchLine(char* file, char* search);

int main(int argc, char *argv[]) 
{

    char* search_pattern = malloc(sizeof(char*)*BUFFERSIZE);

    //if we have at least specify at least 1 file when we call it, we'll have arc-2 files total

    int size = argc-2;
    char* files[size];

    // no search term or files were specified
    if (argc == 1) {
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    } else {
        search_pattern = argv[1];
    }

    // search term specified but not files (use stdin); files[size] no longer relevant
    if (argc == 2) {
        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        line[strcspn(line, "\n")] = 0;
        
        if (strstr(line, search_pattern) != NULL)
        {
            printf("%s\n", line);
        }
    }
    exit(0);
    } else {
        for (int i = 2; i < argc; i++)
        {
            char buffer[BUFFERSIZE];
            strcpy(buffer, argv[i]);
            buffer[strcspn(buffer, "\n")] = 0;
            files[i-2] = argv[i];
        }
    }

    for(int i = 0; i < size; ++i)
    {   
        char* currentFile = files[i];
        searchLine(currentFile, search_pattern);   
    }

    exit(0);
}
    


void searchLine(char* file, char* search)
{
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        printf("wgrep: cannot open file\n");
        exit(1);
    }

    //loop thorugh each line 
    size_t bufferSize = 0;
    char *line = NULL;
    
    while(getline(&line, &bufferSize, fp) != -1)
    {
        if (strstr(line, search) != NULL)
        {
            printf("%s", line);
        }
    }
    free(line);
    fclose(fp);
}