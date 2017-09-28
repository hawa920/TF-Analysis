#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0


const int RK_RHB = 33; /* Rabin-Karp-Rolling-Hash-Base */
const int RK_RHM = ((1 << 30) - 1); /* Rabin-Karp-Rolling-Hash-Moduler */


__attribute__((always_inline)) inline int myabs(const char *src)
{
  return (*src) > 0 ? (*src) : -(*src);
}


__attribute__((always_inline)) inline long long get_hv(const int l_ofst, const int r_ofst, const long long *prefix_ptr, const int *basetab_ptr)
{
  if(l_ofst == FALSE)
    return prefix_ptr[r_ofst];
  long long ret = prefix_ptr[r_ofst] - (prefix_ptr[l_ofst - 1] * basetab_ptr[r_ofst - l_ofst + 1] & RK_RHM);
  return ret > 0 ? ret : (ret + RK_RHM + 1);
}


int main(int argc, char **argv)
{

  FILE *fd = fopen(argv[1], "r");
  assert(fd != NULL);
  assert(fseek(fd, 0, SEEK_END) == FALSE);
  int fsize = (int) ftell(fd);
  assert(fsize != -1 && fsize >= 0);
  assert(fseek(fd, 0, SEEK_SET) == FALSE);

  char *text_ptr = (char *) malloc(sizeof(char) * fsize);
  assert(text_ptr != NULL);
  fread(text_ptr, (size_t) fsize, (size_t) 1, fd);

  /* Rabin-Karp-Rolling-Hash-Initialize */

  int i;
  int *basetab_ptr = (int *) malloc(sizeof(int) * fsize);
  assert(basetab_ptr != NULL);
  long long *prefix_ptr = (long long *) malloc(sizeof(long long) * fsize);
  assert(prefix_ptr != NULL);

  basetab_ptr[0] = 1;
  for(i = 1; i < fsize; i++)
    basetab_ptr[i] = (basetab_ptr[i - 1] * RK_RHB) & RK_RHM;

  prefix_ptr[0] = myabs(&text_ptr[0]);
  for(i = 1; i < fsize; i++)
    prefix_ptr[i] = (prefix_ptr[i - 1] * RK_RHB + myabs(&text_ptr[i])) & RK_RHM;


  /* test searching key-patterns O(N) */

  struct timeval sp, ep;
  char pat[1024];
  printf("Enter pattern to search ...\n");
  scanf("%s", pat);
  int slen = (int) strlen(pat);
  long long hv_pat = 0;
  for(i = 0; i < slen; i++)
    hv_pat = (hv_pat * RK_RHB + myabs(&pat[i])) & RK_RHM;

  gettimeofday(&sp, NULL);
  for(i = 0; i <= fsize - slen; i++)
  {
    if(hv_pat == get_hv(i, i + slen - 1, &prefix_ptr[0], &basetab_ptr[0]))
    {
      printf("Found pattern at text[%d]\n", i);
    }
    else
      ;
  }
  gettimeofday(&ep, NULL);
  printf("O(n) search time cost: %f secs\n", ep.tv_sec - sp.tv_sec + (float) (ep.tv_usec - sp.tv_usec) / 1000000);

  /* close and free */
  assert(fclose(fd) == FALSE);
  free(text_ptr);
  free(basetab_ptr);
  free(prefix_ptr);
  return 0;
}


