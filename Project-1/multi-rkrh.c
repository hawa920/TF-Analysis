#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>


/*---------Constant define--------
 * Rabin-Karp rolling hash base,
 * Rabin-Karp rolling hash moduler,
 * Size of the hash table,
 * -------------------------------*/
const int RK_RHB = 31;
const int RK_RHM = ((1 << 22) - 1);
const int HTSIZE = ((1 << 22) - 1);


/* ----------------------------
 * Nodes designed for linkedlist,
 * use when collisions occur.
 * ----------------------------*/
typedef struct dt_linkedlist
{
  int counter;
  char *key;
  struct dt_linkedlist *p_next;

} dt_linkedlist;


/*------------------------------
 * Nodes designed for hashtable
 * -----------------------------*/
typedef struct dt_hashtable
{
  int list_len;
  struct dt_linkedlist *head, *tail;

} dt_hashtable;



/*------------------------------
 * pthread passing arguments type
 * -----------------------------*/
typedef struct dt_pthread
{
  int key_len;
  char *text_ptr;
  struct dt_hashtable **ptht;

} dt_pthread;


typedef long long lld;


/*
 * ----------------------Description-------------------------*
 * Build up a hash table and insert the keys inside it
 * according to the hash value of the keys.
 * ---------------------Time Consuming-----------------------*
 * Calculating hash value -> constant,
 * Finding corresponded buckets -> constant,
 * Filter duplicate keys -> constant (depends on collision),
 * ----------------------------------------------------------*
 */

dt_hashtable **Key_Hashing(const char *key_path)
{
  FILE *fd = fopen(key_path, "r");
  /* debugger */
  assert(fd != NULL);

  int key_len, idx_i, idx_j;
  lld key_hv;
  dt_linkedlist *p_linkedlist;
  char buf[32], exist;


  dt_hashtable **ptht = (dt_hashtable **) malloc(sizeof(dt_hashtable *) * (HTSIZE + 1));
  /* debugger */
  assert(ptht != NULL);

  for(idx_i = 0; idx_i < (HTSIZE + 1); idx_i++)
  {
    ptht[idx_i] = NULL;
  }


  while(fscanf(fd, "%s", buf) != EOF)
  {
    key_len = strlen(buf);
    key_hv = 0;

    for(idx_i = 0; idx_i < key_len; idx_i++)
    {
      key_hv = (key_hv * RK_RHB + buf[idx_i]) & RK_RHM;
      /* debugger */
      assert(key_hv >= 0);
    }

    /* if the bucket is empty */
    if(ptht[key_hv] == NULL)
    {
      ptht[key_hv] = (dt_hashtable *) malloc(sizeof(dt_hashtable));
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

    /* if the bucket isn't empty */
    else
    {
      exist = '0'; /* filter duplicated keys */
      p_linkedlist = ptht[key_hv] -> head;

      /* check if the key duplicates */
      while(p_linkedlist != NULL)
      {
        if(strcmp(p_linkedlist -> key, buf) == 0)
        {
          exist = '1';
          break;
        }
        p_linkedlist = p_linkedlist -> p_next;
      }

      if(exist == '1')
        continue;

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

/* -----------------Function Description-------------------
 *
 * Read the whole text file at once, and return the pointer
 * which points to the address of the char buffer.
 *
 */

char *Get_File_Content(const char *text_path)
{
  FILE *fd = fopen(text_path, "r");
  /* debugger */
  assert(fd != NULL);

  int ret, fsize;
  ret = fseek(fd, 0, SEEK_END);
  /* debugger */
  assert(ret != -1);

  fsize = (int) ftell(fd);
  /* debugger */
  assert(fsize > 0);

  ret = fseek(fd, 0, SEEK_SET);
  /* debugger */
  assert(ret != -1);

  char *text_ptr = (char *) malloc(sizeof(char) * fsize);
  /* debugger */
  assert(text_ptr != NULL);
  fread(text_ptr, fsize, 1, fd);
  fclose(fd);

  return text_ptr;
}


/* -----------------Function Description-------------------
 *
 * Use pthread to count the occurence of keys of length K,
 *
 */

void pthread_counting(void *pstruct)
{
  /* extract data from dt_pthread object */
  dt_hashtable **ptht =((dt_pthread *) pstruct) -> ptht;
  char *text_ptr = ((dt_pthread *) pstruct) -> text_ptr;
  int key_len = ((dt_pthread *) pstruct) -> key_len;

  /* local variables */
  int idx_i, idx_j, bound, fsize;
  char ifsame;
  lld cur_hv, base_power;
  dt_linkedlist *p_linkedlist;

  fsize = (int) strlen(text_ptr);
  /* debugger */
  assert(fsize > 0);

  /* initialize corresponded base-power */
  base_power = 1;
  for(idx_i = 0; idx_i < key_len; idx_i++)
    base_power = (base_power * RK_RHB) & RK_RHM;


  /* pre-hashing, get the first key_len substring's hash value */
  cur_hv = 0;
  for(idx_i = 0; idx_i < key_len; idx_i++)
    cur_hv = (cur_hv * RK_RHB + (int) text_ptr[idx_i]) & RK_RHM;


  /* rolling hash algorithm */
  bound = fsize - key_len;
  for(idx_i = 0; idx_i <= bound; idx_i++)
  {
    if(ptht[cur_hv] != NULL)
    {
      p_linkedlist = ptht[cur_hv] -> head;
      while(p_linkedlist != NULL)
      {
        ifsame = '1';
        for(idx_j = 0; idx_j < key_len; idx_j++)
        {
          if(text_ptr[idx_i + idx_j] != p_linkedlist -> key[idx_j])
          {
            ifsame = '0';
            break;
          }
        }
        if(ifsame == '1')
        {
          p_linkedlist -> counter ++;
          break;
        }

        else
        {
          p_linkedlist = p_linkedlist -> p_next;
          continue;
        }
      }
    }
    /* drop the leading character and add the tailing one */
    cur_hv = (((cur_hv * RK_RHB) + (int) text_ptr[idx_i + key_len]) - (int) text_ptr[idx_i] * base_power) & RK_RHM;
    /* debugger */
    assert(cur_hv >= 0);
  }
}


/* -------Function Description--------
 *
 * Print out the counting result
 *
 */

void Show_Counting_Result(dt_hashtable **ptht)
{

  dt_linkedlist *p_linkedlist;
  int idx_i, idx_j;

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
      continue;
    }
  }
}


/* ------------Function Description---------------
 *
 * Free all dynamic allocations, including strdup
 *
 */

void Free_Dynamic(dt_hashtable **ptht, char *text_ptr)
{
  int idx_i, idx_j;
  dt_linkedlist *p, *q;
  for(idx_i = 0; idx_i < (HTSIZE + 1); idx_i++)
  {
    if(ptht[idx_i] != NULL)
    {
      for(p = ptht[idx_i] -> head; p != NULL;)
      {
        q = p;
        free(p -> key);
        p = p -> p_next;
        free(q);
      }
      free(ptht[idx_i]);
    }
  }
  free(ptht);
  free(text_ptr);
}


int main(int argc, char **argv)
{

  const char *key_path = *(argv + 1);
  const char *txt_path = *(argv + 2);
  const int pthread_count = 6; /* 2 ~ 7 wchar long */


  int idx_i, idx_j;
  char *text_ptr;
  dt_hashtable **ptht; /* ptr to hashtable */
  dt_pthread pstruct[pthread_count];
  pthread_t pid[pthread_count];


  ptht = Key_Hashing(key_path); /* Read keys and insert into hashtable */
  text_ptr = Get_File_Content(txt_path); /* Read all contents of the text file */


  /* Initialize pthread struct */
  for(idx_i = 0, idx_j = 6; idx_i < pthread_count; idx_i++, idx_j += 3)
  {
    pstruct[idx_i].ptht = ptht; /* pointer to hash table */
    pstruct[idx_i].text_ptr = text_ptr; /* pointer to text buffer */
    pstruct[idx_i].key_len = idx_j; /* the length of the handling keys */
  }


  /* create pthread */
  for(idx_i = 0; idx_i < pthread_count; idx_i++)
  {
    pthread_create(&pid[idx_i], NULL, (void *)pthread_counting, &pstruct[idx_i]);
  }


  /* wait for join */
  for(idx_i = 0; idx_i < pthread_count; idx_i++)
  {
    pthread_join(pid[idx_i], NULL);
  }

  Show_Counting_Result(ptht);
  Free_Dynamic(ptht, text_ptr);
  return 0;
}
