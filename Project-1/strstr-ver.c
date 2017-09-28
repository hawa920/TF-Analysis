#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define TRUE 1
#define FALSE 0

int main(int argc, char ** argv)
{

  FILE *fd = fopen(argv[1], "r");
  assert(fd != NULL);
  assert(fseek(fd, 0, SEEK_END) == FALSE);

  /* get size of file */
  int fsize = (int) ftell(fd);
  assert(fsize != -1 && fsize >= 0);
  assert(fseek(fd, 0, SEEK_SET) == FALSE);

  /* read entire file */
  char *text_ptr = (char *) malloc(sizeof(char) * fsize);
  assert(text_ptr != NULL);
  fread(text_ptr, (size_t) fsize, (size_t) 1, fd);

  char pat[(int) 1<<10];
  printf("Enter the pattern to search ...\n");
  scanf("%s", pat);

  /* strstr seaching */
  char *p = text_ptr;
  while((p = strstr(p, pat)) != NULL)
  {
    printf("Found pattern at text[%d]\n", (int) (p - text_ptr));
    p++;
  }

  return 0;
}
