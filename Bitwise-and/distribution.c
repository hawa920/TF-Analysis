#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main()
{

  int n, a, b, m, i;
  printf("Number of test cases and base modulor: ");
  scanf("%d%d", &n, &m);


  srand(time(NULL));

  for(i = 0; i < n; i++)
  {
    a = rand();
    a = (rand() & 1) ? a : (-a);
    b = rand();
    b = (rand() & 1) ? b : (-b);

    if(((a + b) & m) != ((a & m) + (b & m) & m))
    {
      printf("No.\n");
      return 0;
    }
  }

  printf("Yes.\n");
  return 0;
}
