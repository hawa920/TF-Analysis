/*
 *
 * Pattern matching using Rabin-Karp Rolling Hash
 *    theorical time complexity O(N+m)
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define MAX_PAT_LEN 1024


typedef long long lld;


int main(int argc, char **argv)
{

  FILE *fd = fopen((*(argv + 1)), "r");
  /* see if fopen works */
  assert(fd != NULL);


  const int RK_RHB = 31; /* define Rabin-Karp Rolling Hash Base */
  const int RK_RHM = ((1 << 30) - 1); /* define Rabin-Karp Rolling Hash Moduler */
  int fsize, ret, pat_len, idx_i, idx_j;
  lld pat_hv = 0, cur_hv = 0, base_power = 1;
  char *text_ptr, pat[MAX_PAT_LEN];


  ret = fseek(fd, 0, SEEK_END);
  /* see if fseek works */
  assert(ret != -1);

  fsize = (int) ftell(fd); /* get the entire file size in bytes */
  ret = fseek(fd, 0, SEEK_SET);
  /* see if fseek works */
  assert(ret != -1);

  text_ptr = (char *) malloc(sizeof(char) * fsize);
  /* see if malloc works */
  assert(text_ptr != NULL);

  ret = (int) fread(text_ptr, fsize, 1, fd);
  fclose(fd);

  printf("Enter pattern to search ...\n");
  scanf("%s", pat);
  pat_len = (int) strlen(pat);

  /* detect overflow */
  assert(pat[pat_len] != '\0');

  /* get the hash value of pattern */
  for(idx_i = 0; idx_i < pat_len; idx_i++)
  {
    pat_hv = (pat_hv * RK_RHB + (int) pat[idx_i]) & RK_RHM;
  }

  /* the the first pat_len length hash value of text */
  for(idx_i = 0; idx_i < pat_len; idx_i++)
  {
    cur_hv = (cur_hv * RK_RHB + (int) text_ptr[idx_i]) & RK_RHM;
  }

  /* get the pow(RK_RHB, pat_len - 1) */
  for(idx_i = 0; idx_i < pat_len; idx_i++)
  {
    base_power = (base_power * RK_RHB) & RK_RHM;
  }

  /* ----------------------------O(N) searching----------------------------------*/

   /* Since the boundary is well defined, there's no way to lead to overflows. */

  for(idx_i = 0; idx_i <= fsize - pat_len; idx_i++)
  {
    if(pat_hv == cur_hv)
    {
      printf("Found pattern at text[%d]\n", idx_i);
    }

    else
      ;

    /* The tricky step here is to use bitwise-& to avoid constantly moduling and calling for math::abs */
    cur_hv = (((cur_hv * RK_RHB) + (int) text_ptr[idx_i + pat_len]) - (int) text_ptr[idx_i] * base_power) & RK_RHM;

    /* detect bugs */
    assert(cur_hv >= 0);
  }

  free(text_ptr);
  return 0;
}
