#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXSIZE (1 << 25)

void makerecord(void *map, size_t len) {
  static unsigned int seed = 0;

  for (int *curr = (int *)map; (void *)curr < map + len; curr++) {
    int r = rand_r(&seed);
    *curr = r;
  }
}

int main(int argc, char **argv) {
  if (argc < 3)
    return -1;
  struct {
    int fd;
    void *map;
    char *fn;
  } w;
  w.fn = argv[1];
  unsigned int size = atoi(argv[2]);
  size = size > MAXSIZE ? MAXSIZE : size;
  if ((w.fd = open(w.fn, O_RDWR | O_CREAT | O_TRUNC,
                   S_IRUSR | S_IROTH | S_IWUSR)) == -1) {
    perror("Error opening write file");
    exit(EXIT_FAILURE);
  }

#define error(s)                                                               \
  {                                                                            \
    perror(s);                                                                 \
    close(w.fd);                                                               \
    exit(EXIT_FAILURE);                                                        \
  }

  if (lseek(w.fd, size - 1, SEEK_SET) == -1 || write(w.fd, "", 1) != 1)
    error("Error lseeking");
  if ((w.map = mmap(0, size, PROT_WRITE, MAP_SHARED, w.fd, 0)) == MAP_FAILED)
    error("Map failed");

  makerecord(w.map, size);

  if (munmap(w.map, size) == -1)
    error("Error un-mapping");

  printf("output: %s\n", w.fn);

  close(w.fd);
  exit(0);
}