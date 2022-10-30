#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> 
#include <fcntl.h>
#include <unistd.h>

struct keyRecord {
    int key; 
    int record[25]; 
}; 

struct keyRecord * records;
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

    int fi = open(argv[1], O_RDONLY);
    int numRecords = size / 100; 
    printf("%d\n", size);
    records = malloc(sizeof(struct keyRecord) * numRecords);
    int *file = mmap(NULL, size, PROT_READ, MAP_SHARED, fi,0); 
    printf("%d\n", file[0]);
    for (int i = 0; i < numRecords; i++){
        int key = file[0];
        printf("%d\n", key);
        records[i].key = key;
        for (int j = 0; j < 25; j++){
            records[i].record[j] = file[j];
        }
        file+= 25;
    }
}

/**
 * Sort method - should it take a dictionary as input and output a dictionary too? 
*/
