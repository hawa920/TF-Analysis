#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>


/*----------Constants----------
 * Rabin-Karp rolling hash base
 * Rabin-Karp rolling hash modulor
 * Size of the hash table(buckets)
 * ----------------------------*/
const int RK_RHB = 31;
const int RK_RHM = ((1 << 22) - 1);
const int HTSIZE = ((1 << 22) - 1);


/*----------Linkedlist node-----------
 * Use to deal with the hash collisions
 * -----------------------------------*/
typedef struct dt_linkedlist
{
  int counter;
  char *key;
  struct dt_linkedlist *p_next;

} dt_linkedlist;


/*----------Bucket node----------
 * Main structure of the hash table
 * ------------------------------*/
typedef struct dt_hashtable
{
  int list_len;
  struct dt_linkedlist *head, *tail;

} dt_hashtable;


/*---------pthread struct----------
 * passing arguments of multi-thread
 * --------------------------------*/
typedef struct dt_pthread
{
  char *text_ptr;
  struct dt_hashtable **ptht;

} dt_pthread;


typedef long long lld;


/*----------pre-hash the keys----------
 * Insert keys into hashtable and return
 * a pointer to the starting address of it
 * ------------------------------------*/
dt_hashtable **key_hashing(const char *key_path)
{
  FILE *fd = fopen(key_path, "r");
  /* debugger */
  assert(fd != NULL);

  int key_len, idx_i, idx_j;
  lld key_hv;
  dt_linkedlist *p_linkedlist;
  char buf[256], exist;


  dt_hashtable **ptht = (dt_hashtable **) malloc(sizeof(dt_hashtable *) * (HTSIZE + 1));
  /* debugger */
  assert(ptht != NULL);

  for(idx_i = 0; idx_i < (HTSIZE + 1); idx_i++)
  {
    ptht[idx_i] = NULL;
  }

  while(fscanf(fd, "%s", buf) != EOF)
  {
    key_len = (int) strlen(buf);
    key_hv = 0;

    for(idx_i = 0; idx_i < key_len; idx_i++)
    {
      /* bitwise & obey distribution law when RK_RHM is power of 2 minus 1 */
      key_hv = (key_hv * RK_RHB + buf[idx_i]) & RK_RHM;
      /* debugger */
      assert(key_hv >= 0);
    }

    /* if the bucket is empty */
    if(ptht[key_hv] == NULL)
    {
      ptht[key_hv] = (dt_hashtable*) malloc(sizeof(dt_hashtable));
      /* debugger */
      assert(ptht[key_hv] != NULL);

      p_linkedlist = (dt_linkedlist *) malloc(sizeof(dt_linkedlist));
      /* debugger */
      assert(p_linkedlist != NULL);

      ptht[key_hv] -> list_len = 1;
      ptht[key_hv] -> head = ptht[key_hv] -> tail = p_linkedlist;

      p_linkedlist -> key = strdup(buf);
      p_linkedlist -> counter = 0;
      p_linkedlist -> p_next = NULL;

    }

    /* if the bucket already exists */
    else
    {
      exist = '0'; /* to filter duplicated keys */
      p_linkedlist = ptht[key_hv] -> head;

      while(p_linkedlist != NULL)
      {
        if(strcmp(p_linkedlist -> key, buf) == 0)
        {
          exist = '1';
          break;
        }

        p_linkedlist = p_linkedlist -> p_next;
      }

      /* found duplicated keys */
      if(exist == '1')
      {
        continue;
      }

      /* found keys unique */
      else
      {
        p_linkedlist = (dt_linkedlist *) malloc(sizeof(dt_linkedlist));
        /* debugger */
        assert(p_linkedlist != NULL);

        p_linkedlist -> key = strdup(buf);
        p_linkedlist -> counter = 0;
        p_linkedlist -> p_next = NULL;

        ptht[key_hv] -> tail -> p_next = p_linkedlist;
        ptht[key_hv] -> tail = ptht[key_hv] -> tail -> p_next;
        ptht[key_hv] -> list_len++;
      }
    }
  }
  fclose(fd);
  return ptht;
}


/*----------Divide the text file into N parts----------
 * After partioning the file by the nearest delimeter,
 * we can easily assign to multi-thread function.
 *----------------------------------------------------*/
char **file_partition(const char *txt_path, const int n_part)
{
  FILE *fd = fopen(txt_path, "r");
  /* debugger */
  assert(fd != NULL && n_part > 0);

  int ret, fsize, avesize, idx_i, idx_j;

  ret = fseek(fd, 0, SEEK_END);
  /* debugger */
  assert(ret != -1);

  fsize = (int) ftell(fd);
  /* debugger */
  assert(fsize > 0);

  ret = fseek(fd, 0, SEEK_SET);
  /* debugger */
  assert(ret != -1);

  avesize = fsize / n_part;
  /* debugger */
  assert(avesize > 0);

  /* assume there's at least 1 delimeter in 20 w-characters */
  const int ESTIMATE_RANGE = 60;
  avesize = avesize + ESTIMATE_RANGE;

  char **ptpt = (char **) malloc(sizeof(char *) * n_part);
  /* debugger */
  assert(ptpt != NULL);

  for(idx_i = 0; idx_i < n_part; idx_i++)
  {
    ptpt[idx_i] = (char *) malloc(sizeof(char) * avesize);
    /* debugger */
    assert(ptpt[idx_i] != NULL);

    fread(ptpt[idx_i], avesize, 1, fd);
    ret = fseek(fd, -ESTIMATE_RANGE, SEEK_CUR);
    /* debugger */
    assert(ret != -1);
  }

  fclose(fd);
  return ptpt;
}


/*----------Counting Word Frequencies----------
 * An implementation of Rabin-Karp rolling hash
 * pattern matching with longest match principle
 * --------------------------------------------*/
void pthread_counting(void *pstruct)
{
  /* Extracting data from pstruct object */
  dt_hashtable **ptht = ((dt_pthread *) pstruct) -> ptht;
  char *text_ptr = ((dt_pthread *) pstruct) -> text_ptr;
  /* set the range of bytes, since pattern is 2 ~ 7 w-char long*/
  const int byte_lower_bnd = 6;
  const int byte_upper_bnd = 21;
  int idx_i, idx_j, idx_k, bnd, text_len;
  lld cur_hv, base_power;
  char ifsame;
  dt_linkedlist *p_linkedlist;


  text_len = (int) strlen(text_ptr);

  for(idx_i = byte_upper_bnd; idx_i >= byte_lower_bnd; idx_i -= 3)
  {
    /* calculating corresponded base-power */
    base_power = 1;
    for(idx_j = 0; idx_j < idx_i; idx_j++)
    {
      base_power = (base_power * RK_RHB) & RK_RHM;
    }

    /* pre-hashing the first idx_i length substring's hash value */
    cur_hv = 0;
    for(idx_j = 0; idx_j < idx_i; idx_j++)
    {
      cur_hv = (cur_hv * RK_RHB + (int) text_ptr[idx_j]) & RK_RHM;
    }

    bnd = text_len - idx_i;
    ifsame = '0';

    for(idx_j = 0; idx_j <= bnd; idx_j++)
    {
      if(ptht[cur_hv] != NULL)
      {
        p_linkedlist = ptht[cur_hv] -> head;
        while(p_linkedlist != NULL)
        {
          ifsame = '1';
          for(idx_k = 0; idx_k < idx_i; idx_k++)
          {
            if(text_ptr[idx_j + idx_k] != p_linkedlist -> key[idx_k])
            {
              ifsame = '0';
              break;
            }
          }
          if(ifsame == '1')
          {
            p_linkedlist -> counter++;
            break;
          }

          else
          {
            p_linkedlist = p_linkedlist -> p_next;
            continue;
          }
        }
      }
      if(ifsame == '1')
      {
        ifsame = '0';
        for(idx_k = 0; idx_k < idx_i; idx_k++)
        {
          text_ptr[idx_j + idx_k] = '\0';
        }
        /* re-pre-hash */
        idx_j = idx_j + idx_i;
        cur_hv = 0;
        for(idx_k = 0; idx_k < idx_i; idx_k++)
        {
          cur_hv = (cur_hv * RK_RHB + (int) text_ptr[idx_j + idx_k]) & RK_RHM;
        }
        idx_j--; /* since the for loop will ++idx_j */
        continue;
      }

      /* drop the leading character and add the tailing one */
      else
      {
        cur_hv = (((cur_hv * RK_RHB) + (int) text_ptr[idx_j + idx_i]) - (int) text_ptr[idx_j] * base_power) & RK_RHM;
        /* debugger */
        assert(cur_hv >= 0);
      }
    }
  }
}


/*----------Print out the result----------
 * print out the counting result in the order
 * of the hashtable (hash-value base)
 * ---------------------------------------*/
void show_result(dt_hashtable **ptht)
{
  int idx_i, idx_j;
  dt_linkedlist *p_linkedlist;

  for(idx_i = 0; idx_i < (HTSIZE + 1); idx_i++)
  {
    if(ptht[idx_i] != NULL)
    {
      for(p_linkedlist = ptht[idx_i] -> head; p_linkedlist != NULL; p_linkedlist = p_linkedlist -> p_next)
      {
        printf("%s:\t%d\n", p_linkedlist -> key, p_linkedlist -> counter);
      }
    }

    else
    {
      ;
    }
  }
}


/*----------Free dynamic allocations----------
 * free the memories that are created dynamically,
 * hashtable, pointer to pointer to char, etc.
 * -------------------------------------------*/
void free_dynamic(dt_hashtable **ptht, char **ptpt, int n_part)
{
  int idx_i, idx_j;
  dt_linkedlist *p, *q;
  for(idx_i = 0; idx_i < (HTSIZE + 1); idx_i++)
  {
    if(ptht[idx_i] != NULL)
    {
      for(p = ptht[idx_i] -> head; p != NULL; )
      {
        q = p;
        p = p -> p_next;
        free(q -> key);
        free(q);
      }
      free(ptht[idx_i]);
    }
  }
  free(ptht);

  for(idx_i = 0; idx_i < n_part; idx_i++)
  {
    if(ptpt[idx_i] != NULL)
    {
      free(ptpt[idx_i]);
    }
  }
  free(ptpt);
}


int main()
{
  int n_thread;
  char key_path[256], txt_path[256];
  printf("Format: (key file, text file, number of thread): ...");
  scanf("%s%s%d", key_path, txt_path, &n_thread);


  int idx_i, idx_j;
  char **ptpt;
  dt_hashtable **ptht;
  dt_pthread pstruct[n_thread];
  pthread_t pid[n_thread];


  ptht = key_hashing(&key_path[0]);
  ptpt = file_partition(&txt_path[0], n_thread);

  for(idx_i = 0; idx_i < n_thread; idx_i++)
  {
    pstruct[idx_i].ptht = ptht;
    pstruct[idx_i].text_ptr = ptpt[idx_i];
  }

  for(idx_i = 0; idx_i < n_thread; idx_i++)
  {
    pthread_create(&pid[idx_i], NULL, (void *) pthread_counting, &pstruct[idx_i]);
  }

  for(idx_i = 0; idx_i < n_thread; idx_i++)
  {
    pthread_join(pid[idx_i], NULL);
  }

  return 0;
}
