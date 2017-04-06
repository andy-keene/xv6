// The code for dayofweek was obtained at:
// https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Implementation-dependent_methods
//
#include "types.h"
#include "user.h"
#include "date.h"

static char *months[] = {"NULL", "Jan", "Feb", "Mar", "Apr",
  "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

int
dayofweek(int y, int m, int d)
{
  return (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;
}

int
main(int argc, char *argv[])
{
  int day;
  struct rtcdate r;

  if (date(&r)) {
    printf(2,"Error: date call failed. %s at line %d\n", __FILE__, __LINE__);
    exit();
  }

  day = dayofweek(r.year, r.month, r.day);

  printf(1, "%s %s %d", days[day], months[r.month], r.day);
  printf(1, " %d:%d:%d UTC %d\n", r.hour, r.minute, r.second, r.year);

  exit();
}
