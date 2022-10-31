#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> 
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

struct keyRecord {
    int key; 
    char *record; 
}; 

struct keyRecord * records;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
* todo: set up checks for files + set up output to file
*/
int main(int argc, char *argv[]) {
    FILE* f;
 
    // Opening file in reading mode
    f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END); // seek to end of file
    int size = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); 

    int fi = open(argv[1], O_RDONLY);
    int numRecords = size / 100; 
    records = malloc(sizeof(struct keyRecord) * numRecords);
    char *file = (char *) mmap(NULL, size, PROT_READ, MAP_SHARED, fi,0); 

    for (int i = 0; i < numRecords; i++){
        int key = *((int *)file);
        records[i].key = key;
        records[i].record = malloc(sizeof(char) * 100);
        for (int j = 0; j < 100; j++){
            records[i].record[j] = file[j];
        }
        file+= 100;
    }
    pthread_t p1; 
    pthread_t p2; 
    pthread_t p3; 
    pthread_t p4; 
    pthread_create(&p1, NULL, mergeSort, (void*)0);
    pthread_create(&p2, NULL, mergeSort, (void*)1);
    pthread_create(&p3, NULL, mergeSort, (void*)2);
    pthread_create(&p4, NULL, mergeSort, (void*)3);
    

}

/**
 * Method where the threads start
*/
void* mergeSort(void* args){

}

