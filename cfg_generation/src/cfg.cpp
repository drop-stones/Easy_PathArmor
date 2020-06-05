#include <map>
#include "cfg.hpp"

#define  BUF_SIZE 10

static struct cfg_node *root;
static struct cfg_node *current_node;
static unsigned int cfg_node_id = 0;
static std::map <struct cfg_node *, unsigned int> save_map;
static std::map <unsigned int, struct cfg_node *> load_map;

static void cfg_save_node (std::ofstream &ofs, struct cfg_node *node);
static struct cfg_node *cfg_load_node (std::ifstream &ifs);
static void cfg_free_call (struct cfg_node *caller);
static void cfg_free_node (struct cfg_node *subroot_node);
static struct cfg_node *cfg_find_node (unsigned int id, struct cfg_node *subroot_node);
static struct cfg_node *cfg_find_node (uint64_t entry, struct cfg_node *caller);

/*******************************************************************
 *                       public function                           *
 *******************************************************************/

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
cfg_set_exit (uint64_t exit)
{
  current_node->exit = exit;
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

/* Create new branch node */
struct cfg_node *
cfg_create_branch_node (uint64_t entry, struct cfg_node *parent_node, bool condition)
{
  if (parent_node == NULL) return NULL;

  struct cfg_node *new_node = cfg_find_node (entry, root);
  if (new_node == NULL) /* target node is new! */
    new_node = new struct cfg_node (cfg_node_id++, entry);

#ifdef DEBUG
  printf ("  create Branch-BB%02u: entry = 0x%jx, condition = %d\n", cfg_node_id-1, entry, condition);
#endif

  if (condition)
    parent_node->true_edge  = new_node;
  else
    parent_node->false_edge = new_node;

  current_node = new_node;
  return new_node;
}

/* Create new call node */
struct cfg_node *
cfg_create_call_node (uint64_t call_addr, uint64_t return_addr, uint64_t entry, struct cfg_node *caller)
{
  if (caller == NULL) return NULL;

  struct cfg_node *callee = cfg_find_node (entry, root);
  if (callee == NULL) /* target node is new! */
    callee = new struct cfg_node (cfg_node_id++, entry);

#ifdef DEBUG
  printf ("  create Call-BB%02u: entry = 0x%jx, return = BB%02u\n", cfg_node_id-1, entry, caller->id);
#endif

  caller->call_edges.push_back (std::make_pair (call_addr, callee));
  callee->return_edges.push_back (std::make_pair (return_addr, caller));
  current_node = callee;
  return callee;
}

/* return to latest caller */
struct cfg_node *
cfg_ret (uint64_t exit)
{
  if (current_node->return_edges.size () == 0) return NULL;

#ifdef DEBUG
  printf ("  Return to BB%02u\n", current_node->return_edges.back ().second->id);
#endif

  current_node->exit = exit;
  current_node = current_node->return_edges.back ().second;  /* Latest caller is back () */
  return current_node;
}

/* free All CFG */
void
cfg_free (void)
{
  cfg_free_node (root);
}

/* check whether the path is valid or not */
bool
cfg_check_validity (std::vector <struct cfg_node *> path)
{
  if (path.size () == 0)
    return true;

  struct cfg_node *cfg_ite, *path_ite;
  cfg_ite  = cfg_find_node (path [0]->entry, root);
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

/* save CFG in cfg_file */
void
cfg_save (const char *cfg_file)
{
  std::ofstream ofs (cfg_file);
  if (!ofs) {
    fprintf (stderr, "Error: Cannot open %s\n", cfg_file);
    exit (1);
  }
  cfg_save_node (ofs, root);
  ofs.close ();
}

/* load CFG from cfg_file */
void
cfg_load (const char *cfg_file)
{
  std::ifstream ifs (cfg_file);
  if (!ifs) {
    fprintf (stderr, "Error: Cannot open %s\n", cfg_file);
    exit (1);
  }
  root = cfg_load_node (ifs);
  ifs.close ();
}

/*******************************************************************
 *                      find function                              *
 *******************************************************************/

/* find the call node which has same id */
static struct cfg_node *
cfg_find_call_edges (unsigned int id, struct cfg_node *caller)
{
  struct cfg_node *callee, *true_edge, *false_edge;
  if (caller == NULL) return NULL;
  for (unsigned int i = 0; i < caller->call_edges.size (); i++) {
    callee = caller->call_edges [i].second;
    if (callee->id == id)
      return callee;
    if ((true_edge  = cfg_find_node (id, callee->true_edge))  != NULL)
      return true_edge;
    if ((false_edge = cfg_find_node (id, callee->false_edge)) != NULL)
      return false_edge;
    if ((callee = cfg_find_call_edges (id, callee)) != NULL)
      return callee;
  }
  return NULL;
}

/* find the call node which has same entry */
static struct cfg_node *
cfg_find_call_edges (uint64_t entry, struct cfg_node *caller)
{
  struct cfg_node *callee, *true_edge, *false_edge;
  if (caller == NULL) return NULL;
  for (unsigned int i = 0; i < caller->call_edges.size (); i++) {
    callee = caller->call_edges [i].second;
    if (callee->entry == entry)
      return callee;
    if ((true_edge  = cfg_find_node (entry, callee->true_edge))  != NULL)
      return true_edge;
    if ((false_edge = cfg_find_node (entry, callee->false_edge)) != NULL)
      return false_edge;
    if ((callee = cfg_find_call_edges (entry, callee)) != NULL)
      return callee;
  }
  return NULL;
}

/* find the branch node which has same id */
static struct cfg_node *
cfg_find_node (unsigned int id, struct cfg_node *node)
{
  struct cfg_node *callee, *true_edge, *false_edge;
  if (node == NULL)
    return NULL;
  if (node->id == id)
    return node;
  if ((callee = cfg_find_call_edges (id, node)) != NULL)
    return callee;
  if ((true_edge  = cfg_find_node (id, node->true_edge))  != NULL)
    return true_edge;
  if ((false_edge = cfg_find_node (id, node->false_edge)) != NULL)
    return false_edge;
  return NULL;
}

/* find the branch node which has same entry */
static struct cfg_node *
cfg_find_node (uint64_t entry, struct cfg_node *node)
{
  struct cfg_node *callee, *true_edge, *false_edge;
  if (node == NULL)
    return NULL;
  if (node->entry == entry)
    return node;
  if ((callee = cfg_find_call_edges (entry, node)) != NULL)
    return callee;
  if ((true_edge  = cfg_find_node (entry, node->true_edge))  != NULL)
    return true_edge;
  if ((false_edge = cfg_find_node (entry, node->false_edge)) != NULL)
    return false_edge;
  return NULL;
}

/*******************************************************************
 *                        free function                            *
 *******************************************************************/

static void
cfg_free_call (struct cfg_node *caller)
{
  if (caller == NULL)
    return;

  for (unsigned int i = 0; i < caller->call_edges.size (); i++) {
    cfg_free_call (caller->call_edges [i].second);
  }
  free (caller);
}

static void
cfg_free_node (struct cfg_node *node)
{
  if (node == NULL)
    return;

  cfg_free_node (node->true_edge);
  cfg_free_node (node->false_edge);
  cfg_free_call (node);
}

/*******************************************************************
 *                        save function                            *
 *******************************************************************/

/* check whether the node has already saved or not */
static bool
in_save_map (struct cfg_node *node)
{
  if (save_map.find (node) != save_map.end ())
    return true;
  else
    return false;
}

/* write CFG to ofs recursively */
static void
cfg_save_node (std::ofstream &ofs, struct cfg_node *node)
{
  /* ID: meta data to identify the node (1 origin) */
  static unsigned int ID = 1;
  unsigned int call_count, return_count, i;

  if (node == NULL) {
    ofs << 0 << " ";
    return;
  } else if (in_save_map (node)) {
    /* Only write ID because this is not new node */
    ofs << save_map [node] << " ";
    return;
  }

  ofs << ID << " ";
  save_map [node] = ID;
  ID++;

  ofs << node->id << " ";
  ofs << node->entry << " ";
  ofs << node->exit << " ";
  cfg_save_node (ofs, node->true_edge);
  cfg_save_node (ofs, node->false_edge);

  call_count = node->call_edges.size ();
  ofs << call_count << " ";
  for (i = 0; i < call_count; i++) {
    ofs << node->call_edges [i].first << " ";
    cfg_save_node (ofs, node->call_edges [i].second);
  }

  return_count = node->return_edges.size ();
  ofs << return_count << " ";
  for (i = 0; i < return_count; i++) {
    ofs << node->return_edges [i].first << " ";
    cfg_save_node (ofs, node->return_edges [i].second);
  }
}

/*******************************************************************
 *                        load function                            *
 *******************************************************************/

/* parse unsigned integer from ifs */
static unsigned int
parse_uint (std::ifstream &ifs)
{
  char buffer [BUF_SIZE];
  ifs.getline (buffer, BUF_SIZE, ' ');
  return (unsigned int) atoi (buffer);
}

/* parse uint64_t from ifs */
static uint64_t
parse_uint64_t (std::ifstream &ifs)
{
  char buffer [BUF_SIZE];
  ifs.getline (buffer, BUF_SIZE, ' ');
  return (uint64_t) strtoul (buffer, NULL, 0);
}

/* parse cfg_node from ifs */
static struct cfg_node *
parse_node (std::ifstream &ifs)
{
  unsigned int id;
  uint64_t entry, exit;

  id = parse_uint (ifs);
  if (cfg_node_id < id+1)
    cfg_node_id = id+1;
  entry = parse_uint64_t (ifs);
  exit  = parse_uint64_t (ifs);

  return new struct cfg_node (id, entry, exit);
}

/* check whether the node has already loaded or not */
static bool
in_load_map (unsigned int ID)
{
  if (load_map.find (ID) != load_map.end ())
    return true;
  else
    return false;
}

/* load CFG from ifs recursively */
static struct cfg_node *
cfg_load_node (std::ifstream &ifs)
{
  unsigned int ID, call_count, return_count, i;
  uint64_t call_addr, return_addr;
  struct cfg_node *node;

  ID = parse_uint (ifs);
  if (ID == 0)
    return NULL;
  else if (in_load_map (ID))
    /* already load the node */
    return load_map [ID];

  node = parse_node (ifs);
  load_map [ID] = node;

  node->true_edge  = cfg_load_node (ifs);
  node->false_edge = cfg_load_node (ifs);

  call_count = parse_uint (ifs);
  for (i = 0; i < call_count; i++) {
    call_addr = parse_uint64_t (ifs);
    node->call_edges.push_back (std::make_pair (call_addr, cfg_load_node (ifs)));
  }

  return_count = parse_uint (ifs);
  for (i = 0; i < return_count; i++) {
    return_addr = parse_uint64_t (ifs);
    node->return_edges.push_back (std::make_pair (return_addr, cfg_load_node (ifs)));
  }

  return node;
}

/*******************************************************************
 *                       print function                            *
 *******************************************************************/

static void
cfg_print_node (void)
{
  unsigned int i, j;
  struct cfg_node *cfg_node_i;
  for (i = 0; i < cfg_node_id; i++) {
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
    for (j = 0; j < cfg_node_i->call_edges.size (); j++)
      printf ("BB%02u,", cfg_node_i->call_edges [j].second->id);
    printf ("} return {");
    for (j = 0; j < cfg_node_i->return_edges.size (); j++)
      printf ("BB%02u,", cfg_node_i->return_edges [j].second->id);
    printf ("}\n");
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
      printf (" -> BB%02u", caller->call_edges [i].second->id);
      cfg_print_call_edges (caller->call_edges [i].second, depth+1);
    } else {
      for (unsigned int j = 0; j < depth; j++)
        printf ("        ");
      printf (" -> BB%02u", caller->call_edges [i].second->id);
      cfg_print_call_edges (caller->call_edges [i].second, depth+1);
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
