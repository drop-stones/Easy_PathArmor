#include <stdio.h>
#include <stdlib.h>

void B (void); void D (void); void E (void);

void A (void) { printf ("A->"); B (); printf ("A\n"); }
void B (void) { printf ("B->"); E (); printf ("B->"); }
void C (void) { printf ("C->"); D (); printf ("C\n"); }
void D (void) { printf ("D->"); E (); printf ("D->"); }
void E (void) { printf ("E->"); /* return to D */ }

int
main (int argc, char *argv [])
{
  int x = atoi (argv [1]);
  if ((x % 2) == 0)
    A ();
  else
    C ();
}

