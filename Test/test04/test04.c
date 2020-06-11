#include <stdio.h>
#include <stdlib.h>

void (*fp) ();

void B (void); void D (void); void E (void);

void A () { fp = B; printf ("A->"); (*fp) (); }  void C () { fp = D; printf ("C->"); (*fp) (); }
void B () { fp = E; printf ("B->"); (*fp) (); }  void D () { fp = E; printf ("D->"); (*fp) (); }
void E () { printf ("E\n"); }

int
main (int argc, char *argv [])
{
  int x = atoi (argv [1]);
  if ((x % 2) == 0)
    A ();
  else
    C ();
}
