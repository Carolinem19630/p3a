#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> 
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <sys/types.h>
/*
*   Things that can still be improved: 1) Testing 2) Reallocating work to faster threads 3) Maybe merging the work of the threads? 
*   4) Writing while sorting 5) To do a lot of these improvements I'm wondering if we need a different sorting algo - I think 
*   merge sort makes a lot of sense but it's hard to reallocate work and it has a long critical section
*/
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
    int mid = beg + (end - beg) / 2;
    if (beg < end) {
 
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
    int end = ((numRecords)/numThreads) * (numThread + 1) -1;
    if(numThread + 1 == numThreads){
        end = numRecords - 1;
    }
    sort(beg, end);
    return NULL;
}

// int get_size(char *f){
//     FILE *file = fopen(f, "r");
//     int size = 0;
//     char line[100];
    
//     while(fgets(line, 80, file) != NULL){
//         size+=1;
//     }

//     return size;

// }

int main(int argc, char *argv[]) {
    FILE* f;
 
    // Opening file in reading mode
    f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END); // seek to end of file
    int size = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); 
    int size = get_size(argv[1]);

    int fi = open(argv[1], O_RDONLY, 0666);
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
    // merge all the threads' work - probably could be faster but I'm not sure how if # threads varies at all - it's rounding down is that bad?
    for (int k = 1; k < numThreads; k++){
        int end = (numRecords/numThreads) * (k + 1) -1;
        if(k + 1 == numThreads){
            end = numRecords - 1;
        }
        merge(0, end/2, end);
    }
    merge(0, numRecords/2, numRecords); 
    // write to file
    int fd = open(argv[2], O_RDWR, 0666);
    if (ftruncate(fd, 4096) == 0){
        lseek(fd, 0, SEEK_SET);
        char *map = (char *) mmap(NULL, (numRecords) *100, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0); 
        if (map == MAP_FAILED)
        {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
        for (int l = 0; l < numRecords; l++){
            memcpy(map, records[l].record, 100);
            msync(map, 100, MS_SYNC);
            if (l + 1 != numRecords){
                map +=100;
            }
        }
        fsync(fd);
    }
    close(fd);
}
