#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef unsigned int uint;

/*
 *   Things that can still be improved: 1) Testing 2) Reallocating work to faster threads 3) Maybe merging the work of the threads?
 *   4) Writing while sorting 5) To do a lot of these improvements I'm wondering if we need a different sorting algo - I think
 *   merge sort makes a lot of sense but it's hard to reallocate work and it has a long critical section
 */
struct keyRecord
{
    int key;
    char *record;
};

struct keyRecord *records;

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
        L[i]= records[beg + i];
    for (j = 0; j < n2; j++)
        R[j] = records[mid + 1 + j];

    i = 0;
    j = 0;
    k = beg;
    pthread_mutex_lock(&lock);
    while (i < n1 && j < n2)
    {
        if (L[i].key <= R[j].key)
        {
            records[k] = L[i];
            i++;
        }
        else
        {
            records[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        records[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        records[k] = R[j];
        j++;
        k++;
    }
    pthread_mutex_unlock(&lock);
}

void sort(int beg, int end)
{
    if (beg < end)
    {
        int mid = beg + (end - beg) / 2;

        sort(beg, mid);
        sort(mid + 1, end);

        merge(beg, mid, end);
    }
}

/**
 * Method where the threads start - set beginning and end for the curr thread
 */
void *mergeSort(void *args)
{
    int numThread = *(int *)args;

    int beg = numRecords / numThreads * numThread;
    int end = ((numRecords) / numThreads) * (numThread + 1) - 1;
    if (numThread + 1 == numThreads)
    {
        end = numRecords - 1;
    }
    sort(beg, end);
    return NULL;
}

int main(int argc, char *argv[])
{
    struct
    {
        int fd;
        char *map;
        char *fn;
    } rec, sort;
    rec.fn = argv[1];
    sort.fn = argv[2];

    if ((rec.fd = open(rec.fn, O_RDONLY)) == -1 ||
        (sort.fd = open(sort.fn, O_RDONLY)) == -1)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    stat(rec.fn, &st);
    uint recsize = st.st_size;
    stat(sort.fn, &st);
    uint sortsize = st.st_size;

    if ((rec.map = (char *)mmap(0, recsize, PROT_READ, MAP_SHARED, rec.fd, 0)) ==
            MAP_FAILED ||
        (sort.map = (char *)mmap(0, sortsize, PROT_READ, MAP_SHARED, sort.fd, 0)) ==
            MAP_FAILED)
        exit(EXIT_FAILURE);

    uint size = recsize;
    printf("%d\n", size);
    numRecords = size / 100;
    records = malloc(sizeof(struct keyRecord) * numRecords);

    /* for (int i = 0; i < numRecords; i++){
        int key = *((int *)rec.map);
        records[i].key = key;
        records[i].record = malloc(sizeof(char) * 100);
        for (int j = 0; j < 100; j++){
            records[i].record[j] = rec.map[j];
        }
        rec.map+= 100;
    } */

    struct keyRecord *c = records;

    for (char *r = rec.map; r < rec.map + numRecords * 100; r += 100) {
        c->key = *(int *)r;
        c->record = malloc(sizeof(char) *100);
        memcpy(c->record, r, 100);
        printf("%s\n", c->record);
        c++;
    }

    numThreads = get_nprocs();
    pthread_t p[numThreads];
    int threadNum[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        threadNum[i] = i;
        if (pthread_create(&p[i], NULL, mergeSort, &threadNum[i]) != 0)
        {
        }
    }

    for (int j = 0; j < numThreads; j++)
    {
        pthread_join(p[j], NULL);
    }
    // merge all the threads' work - probably could be faster but I'm not sure how if # threads varies at all - it's rounding down is that bad?
    for (int k = 1; k < numThreads; k++)
    {
        int end = (numRecords / numThreads) * (k + 1) - 1;
        if (k + 1 == numThreads)
        {
            end = numRecords - 1;
        }
        merge(0, end / 2, end);
    }
    // write to file
   /*  int fd = open(argv[2], O_RDWR, 0666);
    if (ftruncate(fd, 4096) == 0)
    {
        lseek(fd, 0, SEEK_SET);
        char *map = (char *)mmap(NULL, (numRecords)*100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED)
        {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        } */
    struct keyRecord *d = records;
    for (int l = 0; l < numRecords; l++)
    {
        memcpy(sort.map, d->record, 100);
        msync(sort.map, 100, MS_SYNC);
        if (l + 1 != numRecords)
        {
            d += 1;
        }
    }
    fsync(sort.fd);  
}
