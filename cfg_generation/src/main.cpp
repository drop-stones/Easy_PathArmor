#include <stdio.h>
#include <stdlib.h>
#include "cfg_generation.hpp"

int
main (int argc, char *argv [])
{
  if (argc < 6) {
    fprintf (stderr, "usage: %s <bin_file> <config_file> <entry_addr> <exit_addr> <save_file>\n", argv [0]);
    exit (1);
  }

  uint64_t entry = (uint64_t) strtoul (argv [3], NULL, 0);
  uint64_t exit  = (uint64_t) strtoul (argv [4], NULL, 0);

  cfg_generation_init ();
  cfg_generation (argv [1], argv [2], entry, exit);
  cfg_save (argv [5]);
  cfg_generation_fini ();
}
