#include <stack>
#include "cfg.hpp"

#define  BUF_SIZE 20

static struct cfg_node *root;
static struct cfg_node *current_node;
static unsigned int cfg_node_id = 0;
static std::map <struct cfg_node *, unsigned int> save_map;
static std::map <unsigned int, struct cfg_node *> load_map;
static std::set <struct cfg_node *> searched_set;
static std::stack <uint64_t> call_stack;


static struct cfg_node *cfg_find_node (unsigned int id, struct cfg_node *node);
static struct cfg_node *cfg_find_node (uint64_t addr, struct cfg_node *node);
static struct cfg_node *cfg_find (unsigned int id);
static struct cfg_node *cfg_find (uint64_t addr);
static void cfg_free_node (struct cfg_node *subroot_node);
static bool check_one_addr_validity (struct cfg_node *check_node, std::deque <uint64_t> path);
static bool check_entry_validity    (struct cfg_node *check_node, std::deque <uint64_t> path);
static bool check_exit_validity     (struct cfg_node *check_node, std::deque <uint64_t> path);
static bool check_call_validity     (struct cfg_node *check_node, std::deque <uint64_t> path);
static bool check_return_validity   (struct cfg_node *check_node, std::deque <uint64_t> path);
static void cfg_save_node (std::ofstream &ofs, struct cfg_node *node);
static struct cfg_node *cfg_load_node (std::ifstream &ifs);

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

struct cfg_node *
cfg_divide_node (struct cfg_node *node, uint64_t entry)
{
  if (node == NULL) return NULL;
  if (!(node->entry < entry && entry < node->exit))
    return node;

  uint64_t call_addr, return_addr;
  std::set <struct cfg_node *> callee_set;
  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::map <uint64_t, struct cfg_node *>::iterator return_itr;
  struct cfg_node *upper, *lower, *return_edge;
  upper = node;
  lower = new struct cfg_node (cfg_node_id++, entry);

  /* lower setting */
  lower->exit       = upper->exit;
  //lower->true_edge  = upper->true_edge;
  lower->true_edges = upper->true_edges;
  lower->false_edge = upper->false_edge;
  /* call/return edges process */
  for (call_itr = upper->call_edges.begin (); call_itr != upper->call_edges.end (); call_itr++) {
    call_addr  = call_itr->first;
    callee_set = call_itr->second;
    if (lower->entry <= call_addr && call_addr <= lower->exit) {
      /** To be fixed: callee return setting **/
      lower->call_edges [call_addr] = callee_set;
      upper->call_edges.erase (call_addr);
    }
  }
  for (return_itr = upper->return_edges.begin (); return_itr != upper->return_edges.end (); return_itr++) {
    return_addr = return_itr->first;
    return_edge = return_itr->second;
    lower->return_edges [return_addr] = return_edge;
    upper->return_edges.erase (return_addr);
  }
  lower->latest_return_edge = upper->latest_return_edge;
  /*** Fault: Cannot get correct exit address ***/
  upper->exit       = entry-1;
  upper->true_edges.clear ();
  upper->false_edge = lower;

  return lower;
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
cfg_jmp_process (uint64_t entry, struct cfg_node *parent_node, bool condition)
{
  if (parent_node == NULL) return NULL;

  struct cfg_node *new_node = cfg_find (entry);
  if (new_node == NULL) /* target node is new! */
    new_node = new struct cfg_node (cfg_node_id++, entry);
  else if (new_node->entry < entry && entry < new_node->exit) /* divide the node */
    new_node = cfg_divide_node (new_node, entry);

#ifdef DEBUG
  printf ("  create Branch-BB%02u: entry = 0x%jx, condition = %d\n", cfg_node_id-1, entry, condition);
#endif

  if (condition)
    //parent_node->true_edge  = new_node;
    parent_node->true_edges.insert (new_node);
  else
    parent_node->false_edge = new_node;

  current_node = new_node;
  return new_node;
}

/* Create new call node */
struct cfg_node *
cfg_call_process (uint64_t call_addr, uint64_t return_addr, uint64_t entry, struct cfg_node *caller)
{
  if (caller == NULL) return NULL;

  struct cfg_node *callee = cfg_find (entry);
  if (callee == NULL) /* target node is new! */
    callee = new struct cfg_node (cfg_node_id++, entry);

#ifdef DEBUG
  printf ("  create Call-BB%02u: entry = 0x%jx, return = BB%02u\n", cfg_node_id-1, entry, caller->id);
#endif

  /* call/return process */
  caller->call_edges [call_addr].insert (callee);
  callee->return_edges [return_addr] = caller;
  callee->latest_return_edge = caller;
  current_node = callee;
  return callee;
}

/* return to latest caller */
struct cfg_node *
cfg_ret_process (uint64_t exit)
{
  if (current_node->return_edges.size () == 0) return NULL;

#ifdef DEBUG
  printf ("  Return to BB%02u\n", current_node->latest_return_edge->id);
#endif

  current_node->exit = exit;
  current_node = current_node->latest_return_edge;
  return current_node;
}

/* free All CFG */
void
cfg_free (void)
{
  searched_set.clear ();
  cfg_free_node (root);
}

/* check whether the path is valid or not */
bool
cfg_check_validity (std::deque <uint64_t> path)
{
  if (path.empty ())
    return true;

  while (!call_stack.empty ())
    call_stack.pop ();
  struct cfg_node *check_node = cfg_find (path.front ());
  return check_one_addr_validity (check_node, path);
}

/* save CFG to cfg_file */
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
  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::set <struct cfg_node *>::iterator callee_itr, edge_itr;
  std::set <struct cfg_node *> callee_set;
  struct cfg_node *callee, *true_edge, *false_edge;
  if (caller == NULL) return NULL;
  for (call_itr = caller->call_edges.begin (); call_itr != caller->call_edges.end (); call_itr++) {
    callee_set = call_itr->second;
    for (callee_itr = callee_set.begin (); callee_itr != callee_set.end (); callee_itr++) {
      callee = *callee_itr;
      if (searched_set.find (callee) != searched_set.end ())
        continue;
      searched_set.insert (callee);
      if (callee->id == id)
        return callee;
      //if ((true_edge  = cfg_find_node (id, callee->true_edge))  != NULL)
      //  return true_edge;
      for (edge_itr = callee->true_edges.begin (); edge_itr != callee->true_edges.end (); edge_itr++) {
        if ((true_edge = cfg_find_node (id, *edge_itr)) != NULL)
          return true_edge;
      }
      if ((false_edge = cfg_find_node (id, callee->false_edge)) != NULL)
        return false_edge;
      if ((callee = cfg_find_call_edges (id, callee)) != NULL)
        return callee;
    }
  }
  return NULL;
}

/* find the call node which has same entry */
static struct cfg_node *
cfg_find_call_edges (uint64_t addr, struct cfg_node *caller)
{
  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::set <struct cfg_node *>::iterator callee_itr, edge_itr;
  std::set <struct cfg_node *> callee_set;
  struct cfg_node *callee, *true_edge, *false_edge;
  if (caller == NULL) return NULL;
  for (call_itr = caller->call_edges.begin (); call_itr != caller->call_edges.end (); call_itr++) {
    callee_set = call_itr->second;
    for (callee_itr = callee_set.begin (); callee_itr != callee_set.end (); callee_itr++) {
      callee = *callee_itr;
      if (searched_set.find (callee) != searched_set.end ())
        continue;
      searched_set.insert (callee);
      if (callee->entry <= addr && addr <= callee->exit)
        return callee;
      //if ((true_edge  = cfg_find_node (addr, callee->true_edge))  != NULL)
      //  return true_edge;
      for (edge_itr = callee->true_edges.begin (); edge_itr != callee->true_edges.end (); edge_itr++) {
        if ((true_edge = cfg_find_node (addr, *edge_itr)) != NULL)
          return true_edge;
      }
      if ((false_edge = cfg_find_node (addr, callee->false_edge)) != NULL)
        return false_edge;
      if ((callee = cfg_find_call_edges (addr, callee)) != NULL)
        return callee;
    }
  }
  return NULL;
}

/* find the branch node which has same id */
static struct cfg_node *
cfg_find_node (unsigned int id, struct cfg_node *node)
{
  struct cfg_node *callee, *true_edge, *false_edge;
  std::set <struct cfg_node *>::iterator edge_itr;
  if (node == NULL)
    return NULL;
  if (searched_set.find (node) != searched_set.end ())
    return NULL;
  searched_set.insert (node);
  if (node->id == id)
    return node;
  if ((callee = cfg_find_call_edges (id, node)) != NULL)
    return callee;
  //if ((true_edge  = cfg_find_node (id, node->true_edge))  != NULL)
  //  return true_edge;
  for (edge_itr = node->true_edges.begin (); edge_itr != node->true_edges.end (); edge_itr++) {
    if ((true_edge = cfg_find_node (id, *edge_itr)) != NULL)
      return true_edge;
  }
  if ((false_edge = cfg_find_node (id, node->false_edge)) != NULL)
    return false_edge;
  return NULL;
}

/* find the branch node which has same entry */
static struct cfg_node *
cfg_find_node (uint64_t addr, struct cfg_node *node)
{
  struct cfg_node *callee, *true_edge, *false_edge;
  std::set <struct cfg_node *>::iterator edge_itr;
  if (node == NULL)
    return NULL;
  if (searched_set.find (node) != searched_set.end ())
    return NULL;
  searched_set.insert (node);
  if (node->entry <= addr && addr <= node->exit)
    return node;
  if ((callee = cfg_find_call_edges (addr, node)) != NULL)
    return callee;
  //if ((true_edge  = cfg_find_node (addr, node->true_edge))  != NULL)
  //  return true_edge;
  for (edge_itr = node->true_edges.begin (); edge_itr != node->true_edges.end (); edge_itr++) {
    if ((true_edge = cfg_find_node (addr, *edge_itr)) != NULL)
      return true_edge;
  }
  if ((false_edge = cfg_find_node (addr, node->false_edge)) != NULL)
    return false_edge;
  return NULL;
}

/* wrapper function to find cfg_node by id.
 * Please use it to find cfg_node.
 */
static struct cfg_node *
cfg_find (unsigned int id)
{
  searched_set.clear ();
  return cfg_find_node (id, root);
}

/* wrapper function to find cfg_node by addr.
 * Please use it to find cfg_node.
 */
static struct cfg_node *
cfg_find (uint64_t addr)
{
  searched_set.clear ();
  return cfg_find_node (addr, root);
}

/*******************************************************************
 *                        free function                            *
 *******************************************************************/

/* free all cfg_node recursively */
static void
cfg_free_node (struct cfg_node *node)
{
  std::set <struct cfg_node *>::iterator edge_itr;
  if (node == NULL)
    return;
  if (searched_set.find (node) != searched_set.end ())
    return;
  searched_set.insert (node);

  //cfg_free_node (node->true_edge);
  for (edge_itr = node->true_edges.begin (); edge_itr != node->true_edges.end (); edge_itr++)
    cfg_free_node (*edge_itr);
  cfg_free_node (node->false_edge);
  node->call_edges.clear ();
  node->return_edges.clear ();
  free (node->latest_return_edge);
  free (node);
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
  unsigned int true_edge_count, call_addr_count, return_count;
  uint64_t call_addr, return_addr;
  struct cfg_node *return_edge;
  std::set <struct cfg_node *>::iterator edge_itr;
  std::set <struct cfg_node *> callee_set;
  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::set <struct cfg_node *>::iterator callee_itr;
  std::map <uint64_t, struct cfg_node *>::iterator return_itr;

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
  //cfg_save_node (ofs, node->true_edge);
  true_edge_count = node->true_edges.size ();
  ofs << true_edge_count << " ";
  for (edge_itr = node->true_edges.begin (); edge_itr != node->true_edges.end (); edge_itr++) {
    cfg_save_node (ofs, *edge_itr);
  }
  cfg_save_node (ofs, node->false_edge);

  call_addr_count = node->call_edges.size ();
  ofs << call_addr_count << " ";
  for (call_itr = node->call_edges.begin (); call_itr != node->call_edges.end (); call_itr++) {
    call_addr  = call_itr->first;
    callee_set = call_itr->second;
    ofs << call_addr << " ";
    ofs << callee_set.size () << " ";
    for (callee_itr = callee_set.begin (); callee_itr != callee_set.end (); callee_itr++) { 
      cfg_save_node (ofs, *callee_itr);
    }
  }

  return_count = node->return_edges.size ();
  ofs << return_count << " ";
  for (return_itr = node->return_edges.begin (); return_itr != node->return_edges.end (); return_itr++) {
    return_addr = return_itr->first;
    return_edge = return_itr->second;
    ofs << return_addr << " ";
    cfg_save_node (ofs, return_edge);
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
  unsigned int ID, true_edge_count, call_addr_count, call_count, return_count, i, j;
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

  //node->true_edge  = cfg_load_node (ifs);
  true_edge_count  = parse_uint (ifs);
  for (i = 0; i < true_edge_count; i++) {
    node->true_edges.insert (cfg_load_node (ifs));
  }
  node->false_edge = cfg_load_node (ifs);

  call_addr_count = parse_uint (ifs);
  for (i = 0; i < call_addr_count; i++) {
    call_addr  = parse_uint64_t (ifs);
    call_count = parse_uint (ifs);
    for (j = 0; j < call_count; j++)
      node->call_edges [call_addr].insert (cfg_load_node (ifs));
  }

  return_count = parse_uint (ifs);
  for (i = 0; i < return_count; i++) {
    return_addr = parse_uint64_t (ifs);
    node->return_edges [return_addr] = cfg_load_node (ifs);
  }

  return node;
}

/*******************************************************************
 *                    verification function                        *
 *******************************************************************/

/* check whether the next addr is valid or not.
 * If so, call a next verification function.
 * If not so, return false.
 */
static bool
check_one_addr_validity (struct cfg_node *check_node, std::deque <uint64_t> path)
{
  if (path.empty ())
    return true;
  if (check_node->entry == path.front ())
    return check_entry_validity (check_node, path);
  if (check_node->exit  == path.front ())
    return check_exit_validity (check_node, path);
  if (check_node->call_edges.find (path.front ()) != check_node->call_edges.end ())
    return check_call_validity (check_node, path);
  if (check_node->return_edges.find (path.front ()) != check_node->return_edges.end ())
    return check_return_validity (check_node, path);

  return false;
}

/* check whether the next addr is entry addr or not.
 * If so, check whether the addr is call_addr or not.
 *   If the addr is call_addr, call check_call_validity ().
 *   If not, pop and call a next verification function.
 */
static bool
check_entry_validity (struct cfg_node *check_node, std::deque <uint64_t> path)
{
  if (path.empty ())
    return true;
  if (check_node->entry != path.front ())
    return false;

#ifdef DEBUG
  printf ("  entry : 0x%jx\n", path.front ());
#endif

  uint64_t entry = path.front ();
  path.pop_front ();
  if (path.empty ())
    return true;
  if (check_node->entry <= path.front () && path.front () <= check_node->exit)
    /* target addr of jmp */
    return check_one_addr_validity (check_node, path);
  else {
    /* call@entry */
    path.push_front (entry);
    return check_call_validity (check_node, path);
  }
}

/* check whether the next addr is exit addr or not.
 * If so, pop and call a next verification function.
 * If not, return false
 */
static bool
check_exit_validity (struct cfg_node *check_node, std::deque <uint64_t> path)
{
  std::set <struct cfg_node *>::iterator edge_itr;
  if (path.empty ())
    return true;
  if (check_node->exit != path.front ())
    return false;

#ifdef DEBUG
  printf ("  exit  : 0x%jx\n", path.front ());
#endif

  path.pop_front ();
  if (path.empty ())
    return true;
  //if (check_node->true_edge  != NULL && check_node->true_edge->entry == path.front ())
  //  return check_entry_validity (check_node->true_edge, path);
  for (edge_itr = check_node->true_edges.begin (); edge_itr != check_node->true_edges.end (); edge_itr++) {
    if ((*edge_itr)->entry == path.front ())
      return check_entry_validity (*edge_itr, path);
  }
  if (check_node->false_edge != NULL && check_node->false_edge->entry == path.front ())
    return check_entry_validity (check_node->false_edge, path);
  if (check_node->exit == path.front ())
    return check_exit_validity (check_node, path);
  if (check_node->call_edges.find (path.front ()) != check_node->call_edges.end ())
    return check_call_validity (check_node, path);
  if (check_node->return_edges.find (path.front ()) != check_node->return_edges.end ())
    return check_return_validity (check_node, path);

  return false;
}

/* check whether the next addr is call addr or not.
 * If so, push the addr to stack and call check_entry_validity ().
 * If not, return false.
 */
static bool
check_call_validity (struct cfg_node *check_node, std::deque <uint64_t> path)
{
  if (path.empty ())
    return true;

  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::set <struct cfg_node *>::iterator callee_itr;
  std::set <struct cfg_node *> callee_set;
  struct cfg_node *callee;
  uint64_t call_addr;
  unsigned int len;

  call_itr = check_node->call_edges.find (path.front ());
  if (call_itr == check_node->call_edges.end ())
    return false;

#ifdef DEBUG
  printf ("  call  : 0x%jx\n", path.front ());
#endif

  call_addr = path.front ();
  path.pop_front ();
  if (path.empty ())
    return true;
  callee_set = call_itr->second;
  for (callee_itr = callee_set.begin (); callee_itr != callee_set.end (); callee_itr++) {
    callee = *callee_itr;
    if (callee->entry == path.front ()) {
      for (len = 2; len <= 6; len++) {
        if (callee->return_edges.find (call_addr + len) != callee->return_edges.end ()) {
          call_stack.push (call_addr + len);
#ifdef DEBUG
  printf ("  push  : 0x%jx\n", call_addr + len);
#endif
          break;
        }
      }
      return check_entry_validity (callee, path);
    }
  }

  return false;
}

/* check whether the next addr is return addr or not.
 * If so, check call/return matching and call a next verification function.
 * If not, return false.
 */
static bool
check_return_validity (struct cfg_node *check_node, std::deque <uint64_t> path)
{
  if (path.empty ())
    return true;

  std::map <uint64_t, struct cfg_node *>::iterator return_itr;
  struct cfg_node *caller;

  return_itr = check_node->return_edges.find (path.front ());
  if (return_itr == check_node->return_edges.end ())
    return  false;

#ifdef DEBUG
  printf ("  return: 0x%jx\n", path.front ());
  printf ("  pop   : 0x%jx\n", call_stack.top ());
#endif

  if (path.front () != call_stack.top ()) /* call/return matching */
    return false;
  path.pop_front ();
  call_stack.pop ();
  if (path.empty ())
    return true;
  caller = return_itr->second;
  if (!(caller->entry <= path.front () && path.front () <= caller->exit))
    return false;

  return check_one_addr_validity (caller, path);
}

/*******************************************************************
 *                       print function                            *
 *******************************************************************/

static void
cfg_print_node (void)
{
  unsigned int i;
  struct cfg_node *cfg_node_i;
  std::set <struct cfg_node *>::iterator edge_itr;
  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::set <struct cfg_node *>::iterator callee_itr;
  std::map <uint64_t, struct cfg_node *>::iterator return_itr;

  for (i = 0; i < cfg_node_id; i++) {
    cfg_node_i = cfg_find (i);
    if (cfg_node_i == NULL)
      continue;
    printf ("BB%02u: 0x%jx - 0x%jx, ", cfg_node_i->id, cfg_node_i->entry, cfg_node_i->exit);
    //if (cfg_node_i->true_edge != NULL)
    //  printf ("true(BB%02u) ", cfg_node_i->true_edge->id);
    //else
    //  printf ("true(NULL) ");
    printf ("true(");
    for (edge_itr = cfg_node_i->true_edges.begin (); edge_itr != cfg_node_i->true_edges.end (); edge_itr++)
      printf ("BB%02u,", (*edge_itr)->id);
    printf (") ");
    if (cfg_node_i->false_edge != NULL)
      printf ("false(BB%02u) ", cfg_node_i->false_edge->id);
    else
      printf ("false(NULL) ");
    printf ("Call {");
    for (call_itr = cfg_node_i->call_edges.begin (); call_itr != cfg_node_i->call_edges.end (); call_itr++)
      for (callee_itr = call_itr->second.begin (); callee_itr != call_itr->second.end (); callee_itr++)
        printf ("BB%02u@0x%jx,", (*callee_itr)->id, call_itr->first);
    printf ("} return {");
    for (return_itr = cfg_node_i->return_edges.begin (); return_itr != cfg_node_i->return_edges.end ();
            return_itr++)
      printf ("BB%02u@0x%jx,", return_itr->second->id, return_itr->first);
    printf ("}\n");
  }
}

static void
cfg_print_tree (struct cfg_node *subroot, unsigned int depth, bool cond)
{
  std::set <struct cfg_node *>::iterator edge_itr;
  if (subroot == NULL)
    return;
  if (searched_set.find (subroot) != searched_set.end ())
    return;
  searched_set.insert (subroot);

  if (cond) {
    printf ("\n");
    for (unsigned int i = 0; i < depth; i++)
      printf ("        ");
  }

  printf ("BB%02u -> ", subroot->id);
  cfg_print_tree (subroot->false_edge, depth+1, false);
  for (edge_itr = subroot->true_edges.begin (); edge_itr != subroot->true_edges.end (); edge_itr++)
    cfg_print_tree (*edge_itr, depth+1, true);
  //cfg_print_tree (subroot->true_edge,  depth+1, true);
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

  std::map <uint64_t, std::set <struct cfg_node *> >::iterator call_itr;
  std::set <struct cfg_node *>::iterator callee_itr;
  std::set <struct cfg_node *> callee_set;
  struct cfg_node *callee;
  for (call_itr = caller->call_edges.begin (); call_itr != caller->call_edges.end (); call_itr++) {
    callee_set = call_itr->second;
    for (callee_itr = callee_set.begin (); callee_itr != callee_set.end (); callee_itr++) {
      callee = *callee_itr;
      if (call_itr == caller->call_edges.begin () && callee_itr == callee_set.begin ()) {
        printf (" -> BB%02u", callee->id);
        cfg_print_call_edges (callee, depth+1);
      } else {
        printf ("    ");
        for (unsigned int i = 0; i < depth; i++)
          printf ("      ");
        printf (" -> BB%02u", callee->id);
        cfg_print_call_edges (callee, depth+1);
      }
    }
  }
}

static void
cfg_print_call_tree (struct cfg_node *subroot)
{
  std::set <struct cfg_node *>::iterator edge_itr;
  if (subroot == NULL)
    return;
  if (searched_set.find (subroot) != searched_set.end ())
    return;
  searched_set.insert (subroot);

  cfg_print_call_edges (subroot, 0);
  cfg_print_call_tree (subroot->false_edge);
  //cfg_print_call_tree (subroot->true_edge);
  for (edge_itr = subroot->true_edges.begin (); edge_itr != subroot->true_edges.end (); edge_itr++)
    cfg_print_call_tree (*edge_itr);
}

void
cfg_print (void)
{
  printf ("*** Basic Block Information ***\n");
  cfg_print_node ();
  printf ("*** CFG Graph ***\n");
  searched_set.clear ();
  cfg_print_tree (root, 0, false);
  printf ("\n");
  printf ("*** Call Graph ***\n");
  searched_set.clear ();
  cfg_print_call_tree (root);
}
