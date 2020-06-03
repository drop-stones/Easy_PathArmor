#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>

struct cfg_node {
  unsigned int id;
  uint64_t entry;
  uint64_t exit;
  struct cfg_node *true_edge;
  struct cfg_node *false_edge;
  std::vector <struct cfg_node *> call_edges;
  struct cfg_node *return_edge;

  cfg_node () : id (0), entry (0), exit (0), true_edge (NULL), false_edge (NULL), return_edge (NULL) {}
  cfg_node (unsigned int id, uint64_t entry)
  {
    this->id          = id;
    this->entry       = entry;
    this->exit        = 0;
    this->true_edge   = NULL;
    this->false_edge  = NULL;
    this->return_edge = NULL;
  }
  cfg_node (unsigned int id, uint64_t entry, uint64_t exit)
  {
    this->id          = id;
    this->entry       = entry;
    this->exit        = exit;
    this->true_edge   = NULL;
    this->false_edge  = NULL;
    this->return_edge = NULL;
  }
  cfg_node (unsigned int id, uint64_t entry, uint64_t exit, struct cfg_node *true_edge,
            struct cfg_node *false_edge, std::vector <struct cfg_node *> call_edges,
            struct cfg_node *return_edge)
  {
    this->id          = id;
    this->entry       = entry;
    this->exit        = exit;
    this->true_edge   = true_edge;
    this->false_edge  = false_edge;
    this->return_edge = return_edge;
  }

  void
  load_setting (unsigned int id, uint64_t entry, uint64_t exit)
  {
    this->id    = id;
    this->entry = entry;
    this->exit  = exit;
  }
};

int cfg_save (const char *cfg_file);
int cfg_load (const char *cfg_file);

struct cfg_node *cfg_get_root (void);
struct cfg_node *cfg_get_current_node (void);
struct cfg_node *cfg_create_root (uint64_t entry);
struct cfg_node *cfg_create_branch_node (uint64_t entry, struct cfg_node *parent_node, bool condition);
struct cfg_node *cfg_create_call_node (uint64_t entry, struct cfg_node *caller);
struct cfg_node *cfg_ret (uint64_t exit);
void   cfg_set_exit_in_current_node (uint64_t exit);
void   cfg_free_all (void);
struct cfg_node *cfg_find_node (unsigned int id, struct cfg_node *subroot_node);
struct cfg_node *cfg_find_node_from_entry (uint64_t entry, struct cfg_node *subroot_node);
bool cfg_check_validity (std::vector <struct cfg_node *> path);
void cfg_print (void);
