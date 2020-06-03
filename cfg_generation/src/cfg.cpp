#include "cfg.hpp"

#define  BUF_SIZE 10

static struct cfg_node *root;
static struct cfg_node *current_node;
static unsigned int cfg_node_id = 0;

static void cfg_save_node (std::ofstream &ofs, struct cfg_node *node);
static void cfg_load_node (std::ifstream &ifs, struct cfg_node **node_ptr);

/*******************************************************************
 *                       public function                           *
 *******************************************************************/

static void
cfg_save_node (std::ofstream &ofs, struct cfg_node *node)
{
  if (node == NULL) return;

  int call_count, i;

  ofs << node->id << " ";
  ofs << node->entry << " ";
  ofs << node->exit << " ";
  if (node->true_edge != NULL) {
    ofs << "0 ";
    cfg_save_node (ofs, node->true_edge);
  } else
    ofs << -1 << " ";
  if (node->false_edge != NULL) {
    ofs << "0 ";
    cfg_save_node (ofs, node->false_edge);
  } else
    ofs << -1 << " ";

  call_count = node->call_edges.size ();
  ofs << call_count << " ";
  for (i = 0; i < call_count; i++) {
    cfg_save_node (ofs, node->call_edges [i]);
  }

  if (node->return_edge != NULL) {
    ofs << node->return_edge->id << " ";
  } else
    ofs << -1 << " ";
}

int
cfg_save (const char *cfg_file)
{
  std::ofstream ofs (cfg_file);
  cfg_save_node (ofs, root);
  ofs.close ();

  return 0;
}

static unsigned int
parse_id (std::ifstream &ifs)
{
  char buffer [BUF_SIZE];
  ifs.getline (buffer, BUF_SIZE, ' ');
  return (unsigned int) atoi (buffer);
}

static uint64_t
parse_addr (std::ifstream &ifs)
{
  char buffer [BUF_SIZE];
  ifs.getline (buffer, BUF_SIZE, ' ');
  return (uint64_t) strtoul (buffer, NULL, 0);
}

static int
parse_flag (std::ifstream &ifs)
{
  char buffer [BUF_SIZE];
  ifs.getline (buffer, BUF_SIZE, ' ');
  return atoi (buffer);
}

static int
parse_call_count (std::ifstream &ifs)
{
  return parse_flag (ifs);
}

static void
cfg_load_node (std::ifstream &ifs, struct cfg_node **node_ptr)
{
  int flag, call_count, i;
  unsigned int id;
  uint64_t entry, exit;
  struct cfg_node *node;

  id = parse_id (ifs);
  if (cfg_node_id < id+1)
    cfg_node_id = id+1;
  entry = parse_addr (ifs);
  exit  = parse_addr (ifs);

  if (*node_ptr == NULL) {
    node = new struct cfg_node (id, entry, exit);
    *node_ptr = node;
  } else {
    /* callee */
    node = *node_ptr;
    node->load_setting (id, entry, exit);
  }

#ifdef DEBUG
  printf ("  create BB%u\n", id);
#endif

  /* '\n' or ' ' ?? */
  flag = parse_flag (ifs);
  if (flag < 0)
    node->true_edge = NULL;
  else
    cfg_load_node (ifs, &node->true_edge);

  flag= parse_flag (ifs);
  if (flag < 0)
    node->false_edge = NULL;
  else
    cfg_load_node (ifs, &node->false_edge);

  call_count = parse_call_count (ifs);
  for (i = 0; i < call_count; i++) {
    struct cfg_node *callee = new struct cfg_node;
    node->call_edges.push_back (callee);
    cfg_load_node (ifs, &callee);
  }

  flag = parse_flag (ifs);
  if (flag < 0)
    node->return_edge = NULL;
  else {
    node->return_edge = cfg_find_node (flag, root);
    if (node->return_edge == NULL)
      printf ("  ** error ** return_edge@%d == NULL\n", flag);
  }
}

int
cfg_load (const char *cfg_file)
{
  std::ifstream ifs (cfg_file);
  cfg_load_node (ifs, &root);
  ifs.close ();

  return 0;
}


/* return root */
struct cfg_node *
cfg_get_root (void)
{
  return root;
}

/* return current_node */
struct cfg_node *
cfg_get_current_node (void)
{
  return current_node;
}

/* setting exit */
void
cfg_set_exit_in_current_node (uint64_t exit)
{
  current_node->exit = exit;
}

static bool
is_in_cfg (uint64_t entry)
{
  struct cfg_node *searched_node = cfg_find_node_from_entry (entry, root);
  if (searched_node != NULL)
    return true;
  else
    return false;
}

static struct cfg_node *
cfg_add_edge (uint64_t entry, struct cfg_node *parent_node, bool condition)
{
  if (parent_node == NULL) return NULL;

  struct cfg_node *new_edge_node = cfg_find_node_from_entry (entry, root);
  if (condition)
    parent_node->true_edge = new_edge_node;
  else
    parent_node->false_edge = new_edge_node;
  current_node = new_edge_node;
  return new_edge_node;
}

/* Create root node */
struct cfg_node *
cfg_create_root (uint64_t entry)
{
  if (root != NULL) {
    current_node = root;
    return root;
  }
  root = new struct cfg_node (cfg_node_id++, entry);
  current_node = root;
  return current_node;
}

/* Create new CFG node
 *   
 */
struct cfg_node *
cfg_create_branch_node (uint64_t entry, struct cfg_node *parent_node, bool condition)
{
  if (parent_node == NULL) return NULL;

  if (is_in_cfg (entry))
    return cfg_add_edge (entry, parent_node, condition);

  struct cfg_node *new_node = new struct cfg_node (cfg_node_id++, entry);

#ifdef DEBUG
  printf ("  create Branch-BB%02u: entry = 0x%jx, condition = %d\n", cfg_node_id, entry, condition);
#endif

  if (condition)
    parent_node->true_edge  = new_node;
  else
    parent_node->false_edge = new_node;

  current_node = new_node;
  return new_node;
}

struct cfg_node *
cfg_create_call_node (uint64_t entry, struct cfg_node *caller)
{
  if (caller == NULL) {
    fprintf (stderr, "*** Error: can't call ***\n");
    return NULL;
  }

  struct cfg_node *callee = new struct cfg_node (cfg_node_id++, entry);

#ifdef DEBUG
  printf ("  create Call-BB%02u: entry = 0x%jx, return = BB%02u\n", cfg_node_id, entry, caller->id);
#endif

  caller->call_edges.push_back (callee);
  callee->return_edge = caller;
  current_node = callee;
  return callee;
}

struct cfg_node *
cfg_ret (uint64_t exit)
{
  if (current_node->return_edge == NULL) {
    fprintf (stderr, "*** Error: can't return ***\n");
    return NULL;
  }

#ifdef DEBUG
  printf ("  Return to BB%02u\n", current_node->return_edge->id);
#endif

  current_node->exit = exit;
  current_node = current_node->return_edge;
  return current_node;
}

static void
cfg_free_call (struct cfg_node *caller)
{
  if (caller == NULL)
    return;

  for (unsigned int i = 0; i < caller->call_edges.size (); i++) {
    cfg_free_call (caller->call_edges [i]);
  }
  free (caller);
}

static void
cfg_free (struct cfg_node *subroot_node)
{
  if (subroot_node == NULL)
    return;

  cfg_free (subroot_node->true_edge);
  cfg_free (subroot_node->false_edge);
  cfg_free_call (subroot_node);
  //free (subroot_node);
}

void
cfg_free_all (void)
{
  cfg_free (root);
}

struct cfg_node *
cfg_find_call_edges (unsigned int id, struct cfg_node *caller)
{
  struct cfg_node *callee;
  for (unsigned int i = 0; i < caller->call_edges.size (); i++) {
    callee = caller->call_edges [i];
    if (callee->id == id)
      return callee;
    if ((callee = cfg_find_call_edges (id, callee)) != NULL)
      return callee;
  }
  return NULL;
}

struct cfg_node *
cfg_find_node (unsigned int id, struct cfg_node *subroot_node)
{
  if (subroot_node == NULL)  return NULL;
  if (subroot_node->id == id)
    return subroot_node;
  struct cfg_node *callee = cfg_find_call_edges (id, subroot_node);
  if (callee != NULL)
    return callee;
  struct cfg_node *true_subtree  = cfg_find_node (id, subroot_node->true_edge);
  struct cfg_node *false_subtree = cfg_find_node (id, subroot_node->false_edge);
  if (true_subtree != NULL)
    return true_subtree;
  if (false_subtree != NULL)
    return false_subtree;
  return NULL;
}

struct cfg_node *
cfg_find_node_from_entry (uint64_t entry, struct cfg_node *subroot_node)
{
  if (subroot_node == NULL)  return NULL;
  if (subroot_node->entry == entry)
    return subroot_node;
  struct cfg_node *true_subtree  = cfg_find_node_from_entry (entry, subroot_node->true_edge);
  struct cfg_node *false_subtree = cfg_find_node_from_entry (entry, subroot_node->false_edge);
  if (true_subtree != NULL)
    return true_subtree;
  if (false_subtree != NULL)
    return false_subtree;
  return NULL;
}

bool
cfg_check_validity (std::vector <struct cfg_node *> path)
{
  if (path.size () == 0)
    return true;

  struct cfg_node *cfg_ite, *path_ite;
  cfg_ite  = cfg_find_node_from_entry (path [0]->entry, root);
  printf ("  check validity: 0 is checking...\n");
  if (cfg_ite == NULL || cfg_ite->id != path [0]->id)
    return false;
  for (unsigned int i = 1; i < path.size (); i++) {
    printf ("  check validity: %d is checking...\n", i);
    if (i == path.size () - 1)
      return true;
    path_ite = path [i];
    if (cfg_ite->true_edge != NULL && cfg_ite->true_edge->id == path_ite->id)
      cfg_ite = cfg_ite->true_edge;
    else if (cfg_ite->false_edge != NULL && cfg_ite->false_edge->id == path_ite->id)
      cfg_ite = cfg_ite->false_edge;
    else
      return false;
  }
  return true;
}

/*******************************************************************
 *                       print function                            *
 *******************************************************************/

static void
cfg_print_node (void)
{
  struct cfg_node *cfg_node_i;
  for (unsigned int i = 0; i < cfg_node_id; i++) {
    cfg_node_i = cfg_find_node (i, root);
    if (cfg_node_i == NULL)
      continue;
    printf ("BB%02u: 0x%jx - 0x%jx, ", cfg_node_i->id, cfg_node_i->entry, cfg_node_i->exit);
    if (cfg_node_i->true_edge != NULL)
      printf ("true(BB%02u) ", cfg_node_i->true_edge->id);
    else
      printf ("true(NULL) ");
    if (cfg_node_i->false_edge != NULL)
      printf ("false(BB%02u) ", cfg_node_i->false_edge->id);
    else
      printf ("false(NULL) ");
    printf ("Call {");
    for (unsigned int j = 0; j < cfg_node_i->call_edges.size (); j++) {
      printf ("BB%02u,", cfg_node_i->call_edges [j]->id); 
    }
    printf ("} ");
    if (cfg_node_i->return_edge != NULL)
      printf ("return(BB%02u)\n", cfg_node_i->return_edge->id);
    else
      printf ("return(NULL)\n");
  }
}

static void
cfg_print_subtree (struct cfg_node *subroot_node, unsigned int depth, bool cond)
{
  if (subroot_node == NULL)
    return;

  if (cond) {
    printf ("\n");
    for (unsigned int i = 0; i < depth; i++) {
      printf ("        ");
    }
  }

  printf ("BB%02u -> ", subroot_node->id);
  cfg_print_subtree (subroot_node->false_edge, depth+1, false);
  cfg_print_subtree (subroot_node->true_edge, depth+1, true);
}

static void
cfg_print_call_edges (struct cfg_node *caller, unsigned int depth)
{
  if (caller == NULL)
    return;

  if (depth == 0)
    printf ("BB%02u", caller->id);
  if (caller->call_edges.size () == 0) {
    printf ("\n");
    return;
  }

  for (unsigned int i = 0; i < caller->call_edges.size (); i++) {
    if (i == 0) {
      printf (" -> BB%02u", caller->call_edges [i]->id);
      cfg_print_call_edges (caller->call_edges [i], depth+1);
    } else {
      for (unsigned int j = 0; j < depth; j++)
        printf ("        ");
      printf (" -> BB%02u", caller->call_edges [i]->id);
      cfg_print_call_edges (caller->call_edges [i], depth+1);
    }
  }
}

static void
cfg_print_subtree_call_edges (struct cfg_node *subroot_node)
{
  if (subroot_node == NULL)
    return;

  cfg_print_call_edges (subroot_node, 0);
  cfg_print_subtree_call_edges (subroot_node->false_edge);
  cfg_print_subtree_call_edges (subroot_node->true_edge);
}

void
cfg_print (void)
{
  printf ("*** Basic Block Information ***\n");
  cfg_print_node ();
  printf ("*** CFG Graph ***\n");
  cfg_print_subtree (root, 0, false);
  printf ("\n");
  printf ("*** Call Graph ***\n");
  cfg_print_subtree_call_edges (root);
}
