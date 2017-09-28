#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* external functions */
extern char * strdup(const char *_s);

/* function declaration*/
void str_init();
void hash_init();
void search();
long long gethv(int l, int r);
/* global variables */
char *pat, *txt;
long long pat_len, txt_len, pat_hv, *prefix_hv, *basetab;
const int base = 33;
const long long mod = 536870912;


/* codes */
int main(int argc, char ** argv)
{

  str_init();
  hash_init();
  search();
  return 0;
}


void str_init()
{
  txt = strdup("今天我去打籃球，籃球場上人很多，都沒有空的籃球框可以讓我投籃球。");
  pat = strdup("籃球");
  txt_len = (const int) strlen(txt);
  pat_len = (const int) strlen(pat);
}


void hash_init()
{
  int i;
  prefix_hv = malloc(sizeof(long long) * txt_len);
  basetab = malloc(sizeof(long long) * txt_len);

  basetab[0] = 1;
  for(i = 1; i < txt_len; i++)
    basetab[i] = (basetab[i - 1] * base) % mod;

  prefix_hv[0] = abs(txt[0]);
  for(i = 1; i < txt_len; i++)
    prefix_hv[i] = (prefix_hv[i - 1] * base + abs(txt[i])) % mod;


  pat_hv = 0;
  for(i = 0; i < pat_len; i++)
    pat_hv = ((pat_hv * base) + abs(pat[i])) % mod;

}

void search()
{
  int i;
  for(i = 0; i <= txt_len - pat_len; i++)
  {
    if(gethv(i, i + pat_len - 1) == pat_hv)
      printf("found %s at txt[%d]\n", pat, i);
    else
      ;

  }
}

long long gethv(int l, int r)
{

  if(l == 0)
    return prefix_hv[r];

  long long ret = prefix_hv[r] - (prefix_hv[l - 1] * basetab[r - l + 1] % mod);
  return ret > 0 ? ret : ret + mod;
}















