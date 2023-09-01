#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++)
    {
        char *fileName = argv[i];
        char c;

        FILE *fp = fopen(fileName, "r");

        if (fp == NULL)
        {
            printf("wcat: cannot open file\n");
            exit(1);

        }
        while((c=fgetc(fp))!=(char)EOF){printf("%lc", c);}
        fclose(fp);
    }
    exit(0);
}

