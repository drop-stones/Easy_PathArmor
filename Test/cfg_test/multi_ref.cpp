#include <stdio.h>

void A (void) { printf ("A"); }

void func0 (void) { A(); }
void func1 (void) { A(); }
void func2 (void) { A(); }

int
main (void)
{
  func0 ();
  func1 ();
  func2 ();
}
