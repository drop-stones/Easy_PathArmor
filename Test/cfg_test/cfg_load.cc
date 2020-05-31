#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../Triton/cfg_generation.h"

int
main (int argc, char *argv [])
{
  cfg_generation_init ();
  cfg_load ("/home/binary/code/PathArmor/Test/cfg_test/cfg.txt");
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
