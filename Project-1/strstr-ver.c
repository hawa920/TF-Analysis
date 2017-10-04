#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


char key[1024][1024];


int main(int argc, char **argv)
{

  FILE *fd = fopen((*(argv + 1)), "r");
  assert(fd != NULL);

  int keycnt = 0, i, fsize, ret, counter;
  char tempbuf[1024], *text_ptr, *p;


  while(fscanf(fd, "%s", key[keycnt]) != EOF)
  {
    keycnt ++;
  }

  fclose(fd);

  fd = fopen((*(argv + 2)), "r");
  assert(fd != NULL);

  ret = fseek(fd, 0, SEEK_END);
  assert(ret != -1);

  fsize = (int) ftell(fd);

  ret = fseek(fd, 0, SEEK_SET);
  assert(ret != -1);


  text_ptr = (char *) malloc(sizeof(char) * fsize);
  fread(text_ptr, fsize, 1, fd);
  fclose(fd);

  for(i = 0; i < keycnt; i++)
  {
    p = &text_ptr[0];
    counter = 0;
    while((p = strstr(p, key[i])) != NULL)
    {
      counter++;
      p++;
    }
    printf("%s: %d\n", key[i], counter);
  }

  free(text_ptr);
  return 0;
}
