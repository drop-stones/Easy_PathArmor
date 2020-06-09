/*
 * Symbolically execute and create new input to find new path.
 * And execute repeatedly until there are no new input.
 *
 * Uses Triton's symbolic emulation mode.
 */

#define DEBUG 1
#include "cfg_generation.hpp"

static int  cfg_generation_config (char *bin_file, char *config_file);
static int  cfg_generation_reconfig 
               (std::pair<std::map<triton::arch::registers_e, uint64_t>,std::map<uint64_t, uint8_t>> input);
static void one_emulation (uint64_t entry, uint64_t exit);
static void clear_branch_constraints (void);
static void add_inputs (std::pair<std::map<triton::arch::registers_e, uint64_t>,std::map<uint64_t, uint8_t>> input);
static int  parse_new_inputs (void);
static void find_new_input (void);

static Binary bin;
static triton::API engine;
static triton::arch::registers_e ip;
static std::map<triton::arch::registers_e, std::vector<uint64_t>> regs_inputs;
static std::map<uint64_t, uint8_t> mem;
static std::vector<triton::arch::registers_e> symregs;
static std::vector<uint64_t> symmem;
static std::vector<std::map<bool, triton::ast::AbstractNode *>> branch_constraints;
static std::vector<triton::ast::AbstractNode *> constraint_lists;
static std::vector<std::pair<std::map<triton::arch::registers_e, uint64_t>,std::map<uint64_t, uint8_t>>> inputs;

/*******************************************************************
 *                       public function                           *
 *******************************************************************/

/* Initialize function
 * This function must be called first.
 */
int
cfg_generation_init (void)
{
  return 0;
}

/* Generate CFG
 *   This function emulate one path in bin_file from entry to exit,
 *    and create new input that lead to new path.
 *   After that, this function repeat it until there are no new path.
 */
int
cfg_generation (char *bin_file, char *config_file, uint64_t entry, uint64_t exit)
{
  cfg_generation_config (bin_file, config_file);
  for (unsigned int i = 0; i < inputs.size (); i++) {
    cfg_generation_reconfig (inputs [i]);
    one_emulation (entry, exit);
  }
  return 0;
}

/* Finalize function
 *   This function must be called last.
 */
int
cfg_generation_fini (void)
{
  unload_binary (&bin);
  cfg_free ();
  return 0;
}

/*******************************************************************
 *                  configuration function                         *
 *******************************************************************/

/* Set architecture to Triton Engine.
 */
static int
set_triton_arch (void)
{
  if(bin.arch != Binary::BinaryArch::ARCH_X86) {
    fprintf(stderr, "Unsupported architecture\n");
    return -1;
  }

  if(bin.bits == 32) {
    engine.setArchitecture(triton::arch::ARCH_X86);
    ip = triton::arch::ID_REG_EIP;
  } else if(bin.bits == 64) {
    engine.setArchitecture(triton::arch::ARCH_X86_64);
    ip = triton::arch::ID_REG_RIP;
  } else {
    fprintf(stderr, "Unsupported bit width for x86: %u bits\n", bin.bits);
    return -1;
  }

  return 0;
}

/* Configuration Triton Engine
 *   This function is called before first emulation.
 *   Load bin_file, and Set concrete or symbolic value depending on config_file.
 */
static int
cfg_generation_config (char *bin_file, char *config_file)
{
  std::map <triton::arch::registers_e, uint64_t> regs;

  std::string fname (bin_file);
  if(load_binary (fname, &bin, Binary::BIN_TYPE_AUTO) < 0) return -1;

  if (set_triton_arch () < 0) return -1;
  engine.enableMode (triton::modes::ALIGNED_MEMORY, true);

  if(parse_sym_config (config_file, &regs_inputs, &mem, &symregs, &symmem) < 0) return -1;

  if (regs_inputs.size () == 0)
    add_inputs (std::make_pair (regs, mem));

  for (auto regs_input: regs_inputs) {
    regs.clear ();
    triton::arch::registers_e regnum = regs_input.first;
    for (auto regs_concrete_value: regs_input.second) {
      regs [regnum] = regs_concrete_value;
      add_inputs (std::make_pair (regs, mem));
    }
  }

  return 0;
}

/* Reconfiguration Symbolic value
 *   This function reconfigure all symbolic value by new-input.
 */
static int
cfg_generation_reconfig (
                std::pair<std::map<triton::arch::registers_e, uint64_t>,std::map<uint64_t, uint8_t>> input)
{
  std::map <triton::arch::registers_e, uint64_t> new_regs = input.first;
  std::map <uint64_t, uint8_t> new_mem = input.second;
  engine.clearPathConstraints ();
  clear_branch_constraints ();
  engine.concretizeAllRegister ();
  engine.concretizeAllMemory ();
  for (auto &kv: new_regs) {
    triton::arch::Register r = engine.getRegister (kv.first);
    engine.setConcreteRegisterValue (r, kv.second);
#ifdef DEBUG
    printf ("  set %lu to %s\n", kv.second, r.getName ().c_str ());
#endif
  }
  for (auto &kv: new_mem) {
    engine.setConcreteMemoryValue (kv.first, kv.second);
  }
  for (auto regid: symregs) {
    triton::arch::Register r = engine.getRegister (regid);
    engine.convertRegisterToSymbolicVariable (r)->setComment (r.getName ());
  }
  for (auto memaddr: symmem) {
    engine.convertMemoryToSymbolicVariable (triton::arch::MemoryAccess (memaddr, 1))
              ->setComment (std::to_string (memaddr));
  }
  return 0;
}

/*******************************************************************
 *                    emulation function                           *
 *******************************************************************/

/* Emulation function
 *   This function emulate bin_file from start_addr to end_addr.
 *   This emulation cover instructions only in .text section.
 *   If the emulated instruction is control-flow instruction, create new CFG node.
 *   Finally, call find_new_input () to solve branch constraints.
 */
static void
one_emulation (uint64_t start_addr, uint64_t end_addr)
{
  cfg_create_root (start_addr);
  Section *sec = bin.get_text_section ();
  uint64_t pc  = start_addr, return_addr;

  while(sec->contains (pc)) {
    char mnemonic[32], operands[200];
    int len = disasm_one(sec, pc, mnemonic, operands);
    if(len <= 0) exit (1);

#ifdef DEBUG
    printf ("0x%jx: %s %s\n", pc, mnemonic, operands);
#endif

    triton::arch::Instruction insn;
    insn.setOpcode(sec->bytes+(pc-sec->vma), len);
    insn.setAddress(pc);
    return_addr = insn.getNextAddress ();

    /* skip call to another section */
    if (!strncmp (mnemonic, "call", 4)) {
      uint64_t jump_pc = strtoull (operands, NULL, 0);
      if (jump_pc != 0) { /* For direct call */
        if (!sec->contains (jump_pc)) {
          pc = insn.getNextAddress ();
#ifdef DEBUG
  printf ("skip call to 0x%jx\n", jump_pc);
#endif
          continue;
        }
      } else { /* For indirect call */
        triton::arch::registers_e regnum = get_triton_regnum (operands);
        if (regnum != triton::arch::ID_REG_INVALID) {
          if (regnum == triton::arch::ID_REG_RAX)
            printf ("This reg is rax.\n");
          jump_pc = (uint64_t) engine.getConcreteRegisterValue (engine.getRegister (regnum));
          if (!sec->contains (jump_pc)) {
            pc = insn.getNextAddress ();
#ifdef DEBUG
  printf ("skip call to 0x%jx\n", jump_pc);
#endif
            continue;
          }
        }
      }
    }

    engine.processing(insn);
    uint64_t next_pc = (uint64_t)engine.getConcreteRegisterValue(engine.getRegister(ip));

    if (insn.isControlFlow ()) {
      if (insn.isBranch ()) {
        /* jmp */
        cfg_set_exit (pc);
        if (insn.isConditionTaken ())
          cfg_jmp_process (next_pc, cfg_get_current_node (), true);
        else
          cfg_jmp_process (next_pc, cfg_get_current_node (), false);
      } else if (!strncmp (mnemonic, "call", 4)) {
        /* call */
        cfg_call_process (pc, return_addr, next_pc, cfg_get_current_node ());
      } else {
        /* ret */
        cfg_ret_process (pc);
      }
    }

    if(pc == end_addr) {
      cfg_set_exit (pc);
      break;
    }

    pc = next_pc;
  }

  find_new_input ();
}

/*******************************************************************
 *                  branch_constraint function                     *
 *******************************************************************/

static void
create_branch_constraints (void)
{
  const std::vector<triton::engines::symbolic::PathConstraint> &path_constraints = engine.getPathConstraints();
  /* create branch_constraint_pair */
  for(auto &path_constraint: path_constraints) {
    if(!path_constraint.isMultipleBranches()) continue;
    std::map<bool, triton::ast::AbstractNode *> branch_constraint_pair;
    for (auto &branch_constraint: path_constraint.getBranchConstraints ()) {
      bool flag = std::get<0>(branch_constraint);
      triton::ast::AbstractNode *constraint = std::get<3>(branch_constraint);
      branch_constraint_pair [flag] = constraint;
    }
    branch_constraints.push_back (branch_constraint_pair);
  }
}

/* Clear all branch_constraint
 */
static void
clear_branch_constraints (void)
{
  branch_constraints.clear ();
}


/*******************************************************************
 *                  constraint_list function                         *
 *******************************************************************/

/* Confirmation whether the constraint_list is in constraint_lists
 *   If it is, return true.
 *   Otherwise, return false.
 */
static bool
in_constraint_lists (triton::ast::AbstractNode *target_list)
{
  if (std::find (constraint_lists.begin (), constraint_lists.end (), target_list) != constraint_lists.end ())
    return true;
  else
    return false;
}

/* Add the new-constraint_list to constraint_lists
 *   If the constraint_list already exists, do nothing.
 *   Otherwise add it.
 */
static void
add_constraint_list (triton::ast::AbstractNode *constraint_list)
{
  if (constraint_list == NULL)
    return;
  if (in_constraint_lists (constraint_list))
    return;

  constraint_lists.push_back (constraint_list);
}

/* Recursive function that create constraint list
 *   This function append depth-th branch_constraint to original constraint_list.
 *   Because there are two constraint in depth-th branch_constraint (true, false),
 *    this create two new constraint_list.
 *   About true_constraint, call this function with the true_constraint recursively
 *    because (depth+1)-th branch_constraint may exist.
 *   About false_constraint, add the false_constraint to constraint_lists
 *    because there are no additional branch_constraint.
 */
static void
create_constraint_list_recursively (triton::ast::AstContext ast_builder,
                                    triton::ast::AbstractNode *constraint_list,
                                    unsigned int depth)
{
  if (branch_constraints.size () == 0)
    return;
  if (branch_constraints.size () <= depth) {
    add_constraint_list (constraint_list);
    return;
  }

  triton::ast::AbstractNode *true_constraint  = branch_constraints [depth] [true];
  triton::ast::AbstractNode *false_constraint = branch_constraints [depth] [false];

  true_constraint  = ast_builder.land (constraint_list, true_constraint);
  create_constraint_list_recursively (ast_builder, true_constraint, depth+1);

  false_constraint = ast_builder.land (constraint_list, false_constraint);
  add_constraint_list (false_constraint);
}

/* Create constraint_lists
 *   This function initialize constraint_list and call create_constraint_list_recursively ().
 */
static void
create_constraint_lists (triton::ast::AstContext ast_builder)
{
  triton::ast::AbstractNode *constraint_list = ast_builder.equal (ast_builder.bvtrue (), ast_builder.bvtrue ());
  create_constraint_list_recursively (ast_builder, constraint_list, 0);
}


/*******************************************************************
 *                       input function                            *
 *******************************************************************/

/* Confirmation whether the input exists in inputs
 *   If it is, return true.
 *   Otherwise, return false.
 */
static bool
in_inputs (std::pair<std::map<triton::arch::registers_e, uint64_t>,std::map<uint64_t, uint8_t>> input)
{
  if (std::find (inputs.begin (), inputs.end (), input) != inputs.end ())
    return true;
  else
    return false;
}

/* Add new-input to inputs
 *   If the input already exists, do nothing.
 *   Otherwise, add it.
 */
static void
add_inputs (std::pair<std::map<triton::arch::registers_e, uint64_t>,std::map<uint64_t, uint8_t>> input)
{
  if (in_inputs (input))
    return;

  inputs.push_back (input);
}

static void
create_new_inputs (void)
{
  parse_new_inputs ();
}

/* Parse constraint_list and create New-inputs
 *   Caluculate model for all constraint_list in constraint_lists.
 *   Set the model to new_input, and call add_inputs () to add it to inputs.
 */
static int
parse_new_inputs (void)
{
  triton::arch::registers_e triton_reg;

  if (constraint_lists.size () == 0) return 0;
  for (auto &constraint_list: constraint_lists) {
    std::map<triton::arch::registers_e, uint64_t> new_regs;
    std::map<uint64_t, uint8_t> new_mem;
    for (auto &kv: engine.getModel (constraint_list)) {
#ifdef DEBUG
      printf (" SymVar %u (%s) = 0x%jx\n",
              kv.first,
              engine.getSymbolicVariableFromId (kv.first)->getComment().c_str(),
              (uint64_t)kv.second.getValue());
#endif
      const char *key = engine.getSymbolicVariableFromId (kv.first)->getComment ().c_str ();
      triton_reg = get_triton_regnum (key);
      if(triton_reg == triton::arch::ID_REG_INVALID) {
        /* mems */
        /* write later */
      } else {
        /* regs */
        new_regs [triton_reg] = (uint64_t) kv.second.getValue ();
      }
    }
    std::pair<std::map<triton::arch::registers_e, uint64_t>, std::map<uint64_t, uint8_t>>
      new_input = std::make_pair (new_regs, new_mem);
    add_inputs (new_input);
  }

  return 0;
}

/* Find new-inputs
 *   This function is called after one emulation, 
 *   First, this function create the list of branch_constraints.
 *    (Branch_constraints consist of true_constraint and false_constraint.)
 *   Second, call create_constraint_lists () to create constraint_lists from branch_constraints.
 *   Next, call create_new_inputs () to create new-inputs from constraint_lists.
 *   Last, clear branch_constraints.
 */
static void
find_new_input (void)
{
  triton::ast::AstContext &ast_builder = engine.getAstContext();

  create_branch_constraints ();
  create_constraint_lists (ast_builder);
  create_new_inputs ();
}

