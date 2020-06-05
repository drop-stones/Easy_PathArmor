#include <stdio.h>

void A (void) { printf ("A"); };
void B (void) { printf ("B"); };

int
main (void)
{
  int i = 0;
  A ();
_target:
  B ();
  if (i == 0) {
    i++;
    goto _target;
  }
  return 0;
}
