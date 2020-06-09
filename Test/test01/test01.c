#include <stdio.h>
#include <stdlib.h>

void B (void); void D (void); void E (void);

void A (void) { printf ("A->"); B (); }  void C (void) { printf ("C->"); D (); }
void B (void) { printf ("B->"); E (); }  void D (void) { printf ("D->"); E (); }
void E (void) { printf ("E\n"); }


int
main (int argc, char *argv [])
{
  int x = atoi (argv [1]);
  if ((x % 2) == 0)
    A ();
  else
    C ();
}

