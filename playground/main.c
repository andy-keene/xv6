//a playground for c programs
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

    int a[] = {1,2,3};
    char * sys[] = {
[ONE] = "!", 
[TWO] = "1",
[THREE] = "three"};

     printf("%d", ONE);
     for(int i = ONE; i < FOUR+1; i++){
          printf("call, %s, is %d",sys[i],i);
    }


}
