#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv [])
{
  int jump_num = atoi (argv [1]) % 3;
  /* jmp to jump_num label */
_label0:
  puts ("label0");
  goto _return;
_label1:
  puts ("label1");
  goto _return;
_label2:
  puts ("label2");
  goto _return;
_label3:
  puts ("label3 (can't reach!)");
  goto _return;

_return:
  return 0;
}
