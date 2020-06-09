#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <map>
#include <set>
#include <deque>
#include <iostream>
#include <fstream>

struct cfg_node {
  unsigned int id;
  uint64_t entry;
  uint64_t exit;
  //struct cfg_node *true_edge;
  std::set <struct cfg_node *> true_edges;
  struct cfg_node *false_edge;
  std::map <uint64_t, std::set <struct cfg_node *> > call_edges;
  std::map <uint64_t, struct cfg_node *> return_edges;
  struct cfg_node *latest_return_edge;

  cfg_node () : id (0), entry (0), exit (0), false_edge (NULL) {}
  cfg_node (unsigned int id, uint64_t entry)
  {
    this->id                 = id;
    this->entry              = entry;
    this->exit               = 0;
    //this->true_edge          = NULL;
    this->false_edge         = NULL;
    this->latest_return_edge = NULL;
  }
  cfg_node (unsigned int id, uint64_t entry, uint64_t exit)
  {
    this->id                 = id;
    this->entry              = entry;
    this->exit               = exit;
    //this->true_edge          = NULL;
    this->false_edge         = NULL;
    this->latest_return_edge = NULL;
  }
};

void cfg_save (const char *cfg_file);
void cfg_load (const char *cfg_file);

struct cfg_node *cfg_get_root (void);
struct cfg_node *cfg_get_current_node (void);
void   cfg_set_exit (uint64_t exit);
struct cfg_node *cfg_create_root (uint64_t entry);
struct cfg_node *cfg_jmp_process (uint64_t entry, struct cfg_node *parent_node, bool condition);
struct cfg_node *cfg_call_process (uint64_t call_addr, uint64_t return_addr, uint64_t entry, struct cfg_node *caller);
struct cfg_node *cfg_ret_process (uint64_t exit);
void   cfg_free (void);
//bool cfg_check_validity (std::vector <struct cfg_node *> path);
bool cfg_check_validity (std::deque <uint64_t> path);
void cfg_print (void);
