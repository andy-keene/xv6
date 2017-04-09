#include <time.h> //a playground for c programs
#include <unistd.h>
#define ONE    1
#define TWO    ONE+1
#define THREE  TWO+1
#define FOUR   THREE+1

//static char * (*sycalls[])(void) = {
/*
static char * systemcalls[]= {
 [ONE] "one",
 [TWO] "two",
 [THREE] "three",
}
*/
#include<stdio.h>


int
main()
{
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
/*   
   int * int_ptr;
   int_ptr = &int_num;
   int_ptr = &float_num;
   int_ptr = &character;
*/
  
//  printf("p: %x, &p: %x, (void*)&p: %x \n", p, &p, (void*)&p);

}
