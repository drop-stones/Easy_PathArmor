#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../Triton/cfg_generation.h"

int
main (int argc, char *argv [])
{
  if (argc < 5) {
    fprintf (stderr, "usage : %s <bin> <config> <start> <end>\n", argv [0]);
    exit (1);
  }

  uint64_t start = strtoul (argv [3], NULL, 0);
  uint64_t end   = strtoul (argv [4], NULL, 0);

  cfg_generation_init ();
  cfg_generation (argv [1], argv [2], start, end);

  //printf ("*** print CFG ***\n");
  cfg_print ();

  printf ("*** create path ***\n");
  std::vector <struct cfg_node *> path;
  struct cfg_node *ite;
  for (ite = cfg_get_root (); ite != NULL; ite = ite->false_edge) {
    printf ("push_back (BB%02u)\n", ite->id);
    path.push_back (ite);
  }

  printf ("*** checking validity ***\n");
  bool validity = cfg_check_validity (path);
  if (validity)
    printf ("valid\n");
  else
    printf ("invalid\n");

  const char* cfg_file = "cfg.txt";
  cfg_save (cfg_file);

  cfg_generation_fini ();
}
