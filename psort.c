#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * Takes two arguments -> an input file and an output file
 * The input file has records which are each 100 bytes: 
 * We need to read in each 100 bytes into a data structure (maybe a dictionary) and store the first 4 bytes as 
 * the key and the total 100 bytes as the value
*/
int main(int argc, char *argv[]) {
    FILE* f;
    char ch;
 
    // Opening file in reading mode
    f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END); // seek to end of file
    int size = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); 

    int numRecords = size / 100; 
    printf("%d\n", size);
    char buffer[size];
    fread(buffer, 100, numRecords, f);

    for (int i = 0; i < numRecords; i++){
        printf("%ld\n", sizeof(buffer[i]));
        // need to read in first four bytes and save in variable
        // need to store that value in an array
        // need to store key and record in hashtable
    }
}

/**
 * Sort method - should it take a dictionary as input and output a dictionary too? 
*/