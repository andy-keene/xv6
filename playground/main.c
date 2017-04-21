#include <time.h> //a playground for c programs
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uproc.h"

#define uint unsigned int
#define STRMAX 32

int buildtable(int max, struct uproc * table)
{
   for( int i = 0; i < max; i++){
       table[i].size = 100 + i;
       table[i].pid = i;
       strncpy(table[i].name, "Name", 4);
   }
  return 0;
}

int
main()
{
  int max = 10;
//  struct uproc table[max];
  struct uproc * table = (struct uproc*) malloc(sizeof(struct uproc) * max);

  buildtable(max, table); 
  //uproc * table;
  for (int i = 0; i < max; i++){
    printf("%d: table[i].size: %d, table[i].pid: %d, table[i].name: %s \n", i, table[i].size, table[i].pid, table[i].name);
  }
  free(table);



/*   
   int * int_ptr;
   *int_ptr = 1;
   
   int * p;
   *p = 1;

   double * dbl_ptr;
   *dbl_ptr = 2.0;

   int int_num = 1;
   float float_num = 2.0;
   char character = 'a';

   void * void_ptr; 
   void_ptr  = &int_num;
   void_ptr  = &float_num;
   void_ptr  = &character;
   int * int_ptr;
   int_ptr = &int_num;
   int_ptr = &float_num;
   int_ptr = &character;
*/
  
//  printf("p: %x, &p: %x, (void*)&p: %x \n", p, &p, (void*)&p);

}
