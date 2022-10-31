#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> 
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>

struct keyRecord {
    int key; 
    char *record; 
}; 

struct keyRecord * records;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // lock for array of keyRecords

int numThreads;
int numRecords;

void merge(int beg, int mid, int end)
{
    int i, j, k;
    int n1 = mid - beg + 1;
    int n2 = end - mid;
 
    struct keyRecord *L = malloc(sizeof(struct keyRecord) * n1);
    struct keyRecord *R = malloc(sizeof(struct keyRecord) * n2);
 
    for (i = 0; i < n1; i++)
        L[i] = records[beg + i];
    for (j = 0; j < n2; j++)
        R[j] = records[mid + 1 + j];
 
    i = 0; 
    j = 0; 
    k = beg; 
    pthread_mutex_lock(&lock);
    while (i < n1 && j < n2) {
        if (L[i].key <= R[j].key) {
            records[k] = L[i];
            i++;
        }
        else {
            records[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        records[k] = L[i];
        i++;
        k++;
    }
 
    while (j < n2) {
        records[k] = R[j];
        j++;
        k++;
    }
    pthread_mutex_unlock(&lock);
}

void sort(int beg, int end)
{
    if (beg < end) {
        int mid = beg + (end - beg) / 2;
 
        sort(beg, mid);
        sort(mid + 1, end);
 
        merge(beg, mid, end);
    }
}

/**
 * Method where the threads start - set beginning and end for the curr thread
*/
void* mergeSort(void* args){
    int numThread = *(int*) args;

    int beg = numRecords / numThreads * numThread;
    int end = (numRecords/numThreads) * (numThread + 1) -1;
    sort(beg, end);
}

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
    numRecords = size / 100; 
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

    numThreads = get_nprocs();
    pthread_t p[numThreads];
    int threadNum[numThreads];
    for (int i = 0; i < numThreads; i++){
        threadNum[i] = i; 
        if (pthread_create(&p[i], NULL, mergeSort, &threadNum[i]) != 0){
             
        }
    }

    for (int j= 0; j < numThreads; j++){
        pthread_join(p[j], NULL); 
    }
    
    // todo: figure out how to merge all the threads' work

    // todo: write records to file
}
