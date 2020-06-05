#include <stdio.h>
#include <stdlib.h>
#include "/home/binary/code/PathArmor/cfg_generation/src/cfg.hpp"

int
main (int argc, char *argv [])
{
  if (argc < 2) {
    fprintf (stderr, "usage: %s <cfg_file>\n", argv [0]);
    exit (1);
  }

  cfg_load (argv [1]);
  cfg_print ();
  cfg_free ();

  return 0;
}
