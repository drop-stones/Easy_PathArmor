
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "code_coverage.h"

int
main (int argc, char *argv [])
{
  if (argc < 5) {
    printf ("usage: %s <binary> <config> <entry> <branch-addr>\n", argv [0]);
    return 1;
  }

  uint64_t entry = strtoul (argv [3], NULL, 0);
  uint64_t exit  = strtoul (argv [4], NULL, 0);

  cfg_generation_config (argv [1], argv [2]);
  cfg_gen (entry, exit);
  cfg_generation_fini ();

  return 0;
}
