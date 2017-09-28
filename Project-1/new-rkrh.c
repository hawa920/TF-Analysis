#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define MAX_PAT_LEN 1024


typedef long long lld;


__attribute__((always_inline)) inline int myabs(const char *src)
{
  return *src > 0 ? *src : -(*src);
}


int main(int argc, char **argv)
{

  FILE *fd = fopen((*(argv + 1)), "r");
  //
  assert(fd != NULL);
  //

  const int RK_RHB = 31, RK_RHM = ((1 << 30) - 1);
  int fsize, ret, pat_len, idx_i, idx_j;
  lld pat_hv = 0, cur_hv = 0, base_power = 1;
  char *text_ptr, pat[MAX_PAT_LEN];


  ret = fseek(fd, 0, SEEK_END);
  //
  assert(ret != -1);
  //
  fsize = (int) ftell(fd);
  ret = fseek(fd, 0, SEEK_SET);
  //
  assert(ret != -1);
  //
  text_ptr = (char *) malloc(sizeof(char) * fsize);
  //
  assert(text_ptr != NULL);
  //
  ret = (int) fread(text_ptr, fsize, 1, fd);

  fclose(fd);

  printf("Enter pattern to search ...\n");
  scanf("%s", pat);
  pat_len = (int) strlen(pat);

  /* get the hash value of pattern */

  for(idx_i = 0; idx_i < pat_len; idx_i++)
  {
    pat_hv = (pat_hv * RK_RHB + myabs(pat + idx_i)) & RK_RHM;
  }

  /* the the first pat_len length hash value of text */
  for(idx_i = 0; idx_i < pat_len; idx_i++)
  {
    cur_hv = (cur_hv * RK_RHB + myabs(text_ptr + idx_i)) & RK_RHM;
  }

  /* get the pow(RK_RHB, pat_len - 1) */
  for(idx_i = 0; idx_i < pat_len; idx_i++)
  {
    base_power = (base_power * RK_RHB) & RK_RHM;
  }


  /* O(N) searching */


  for(idx_i = 0; idx_i <= fsize - pat_len; idx_i++)
  {
    if(pat_hv == cur_hv)
    {
      printf("Found pattern at text[%d]\n", idx_i);
    }

    else
      ;

    cur_hv = ((cur_hv * RK_RHB)) + myabs(text_ptr + idx_i + pat_len);
    cur_hv = cur_hv - ((myabs(text_ptr + idx_i) * base_power));
    cur_hv = cur_hv > 0 ? cur_hv : cur_hv + RK_RHM + 1;
    cur_hv = cur_hv & RK_RHM;

  }

/*
    if(pat_hv == cur_hv)
  {
    printf("Found pattern at text[%d]\n", idx_i);
  }
*/
  free(text_ptr);
  return 0;
}
