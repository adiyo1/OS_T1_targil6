#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    FILE *fp = fopen("phonebook.txt", "a"); // Replace with your filename
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    int i;
    fprintf(fp, "%s", argv[1]);
    for (i = 2; i < argc - 1; i++)
    {
        fprintf(fp, " %s", argv[i]);
    }

    fprintf(fp, ",%s\n", argv[i]);

    fclose(fp);
    return 0;
}