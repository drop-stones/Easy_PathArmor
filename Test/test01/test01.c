#include <stdio.h>
#include <stdlib.h>

void A (void);
void B (void);
void C (void);
void D (void);
void E (void);

void
A (void)
{
  printf ("A -> ");
  B ();
}

void
B (void)
{
  printf ("B -> ");
  E ();
}

void
C (void)
{
  printf ("C -> ");
  D ();
}

void
D (void)
{
  printf ("D -> ");
  E ();
}

void
E (void)
{
  printf ("E\n");
}

int
main (int argc, char *argv [])
{
  if (argc < 2) {
    fprintf (stderr, "usage: %s <num>\n", argv [0]);
    exit (1);
  }

  int x = atoi (argv [1]);
  if ((x % 2) == 0)
    A ();
  else
    C ();

  return 0;
}

