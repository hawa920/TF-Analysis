#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>


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
 * Given the length of the key,
 * the program will compare the string using
 * the hash value in constant time
 * --------------------Time Consuming----------------------
 *  Calculating hash value -> constant time,
 *  Finding out corresponded buckets -> O(1),
 *  Traverse linkedlist if there's collision -> constant,
 *  Check if the strings are the same-> O(strlen)
 *  ------------------------------------------------------*/
void Key_Counting(const char *txt_path, dt_hashtable **ptht)
{
  FILE *fd = fopen(txt_path, "r");
  /* debugger */
  assert(fd != NULL);

  const int byte_low_bnd = 6, byte_up_bnd = 21;
  int fsize, idx_i, idx_j, idx_k, ret, bound;
  char *text_ptr, ifsame;
  lld cur_hv, base_power;
  dt_linkedlist *p_linkedlist;

  ret = fseek(fd, 0, SEEK_END);
  /* debugger */
  assert(ret != -1);

  fsize = (int) ftell(fd); /* get entire file size */
  ret = fseek(fd, 0, SEEK_SET);
  /* debugger */
  assert(ret != -1);

  text_ptr = (char *) malloc(sizeof(char) * fsize);
  /* debugger */
  assert(text_ptr != NULL);

  ret = (int) fread(text_ptr, fsize, 1, fd);
  fclose(fd);


  /* Counting occurence of keys */
  for(idx_i = byte_up_bnd; idx_i >= byte_low_bnd; idx_i -= 3)
  {

    /* calculating corresponded base-power */
    base_power = 1;
    for(idx_j = 0; idx_j < idx_i; idx_j++)
      base_power = (base_power * RK_RHB) & RK_RHM;


    /* pre-hashing, get the first idx_i length substring's hash value */
    cur_hv = 0;
    for(idx_j = 0; idx_j < idx_i; idx_j++)
      cur_hv = (cur_hv * RK_RHB + (int) text_ptr[idx_j]) & RK_RHM;


    /* rolling hash algorithm */
    bound = fsize - idx_i;
    ifsame = '0';

    for(idx_j = 0; idx_j <= bound; idx_j++)
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

      /* longest match implementations */
      if(ifsame == '1')
      {
        ifsame = '0';
        for(idx_k = 0; idx_k < idx_i; idx_k++)
          text_ptr[idx_j + idx_k] = '\0';

        /* re-prehashing */
        idx_j = idx_j + idx_i;
        cur_hv = 0;
        for(idx_k = 0; idx_k < idx_i; idx_k++)
          cur_hv = (cur_hv * RK_RHB + (int) text_ptr[idx_j + idx_k]) & RK_RHM;

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
  free(text_ptr);
}


/*------------------------------------
 *  Print out the counting result
 * ---------------------------------*/
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


/* ----------------------------------------------
 * Free all dynamic allocatingm, including strdup
 * ----------------------------------------------*/
void Free_Hashing(dt_hashtable **ptht)
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
}


int main(int argc, char **argv)
{
  const char *key_path = *(argv + 1);
  const char *txt_path = *(argv + 2);
  dt_hashtable **ptht; /* ptr to hashtable */
  ptht = Key_Hashing(key_path);
  Key_Counting(txt_path, ptht);
  Show_Counting_Result(ptht);
  Free_Hashing(ptht);
  return 0;
}
