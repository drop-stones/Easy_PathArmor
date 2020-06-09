#include <stdio.h>
#include <stdlib.h>

void func0 (void) { puts ("func0"); }
void func1 (void) { puts ("func1"); }
void func2 (void) { puts ("func2"); }
void func3 (void) { puts ("func3 (can't call)"); }

void (* func [4]) (void) = { func0, func1, func2, func3 };

int
main (int argc, char * argv [])
{
  int index = atoi (argv [1]);
  func [index % 3] ();
}
