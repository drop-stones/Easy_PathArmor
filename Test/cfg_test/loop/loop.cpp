#include <stdio.h>

void A (void) { printf ("A"); }

int
main (void)
{
  for (int i = 0; i < 100; i++)
    A ();
  return 0;
}
