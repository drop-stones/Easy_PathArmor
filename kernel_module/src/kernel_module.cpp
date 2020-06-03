#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "pin.H"
#include "/home/binary/code/PathArmor/cfg_generation/src/cfg.hpp"

uint64_t start_addr;
uint64_t end_addr;
bool     record_flag;
std::vector<uint64_t> call_stack;
std::vector<struct cfg_node *> path;

/*****************************************************************************
 *                             Analysis functions                            *
 *****************************************************************************/


/* 
 * 
 */
static void
branch_record (ADDRINT bb_exit, ADDRINT next_bb_entry)
{
  /*
  PIN_LockClient ();

  IMG now_img, next_img;
  now_img  = IMG_FindByAddress (bb_exit);
  next_img = IMG_FindByAddress (next_bb_entry);
  if (!IMG_Valid (now_img) || !IMG_Valid (next_img))
    return;
  if (!IMG_IsMainExecutable (now_img) && !IMG_IsMainExecutable (next_img))
    return;

  PIN_UnlockClient ();
  */

  //if (path.back ()->exit != bb_exit)
 /* struct cfg_node *root = cfg_get_root ();
  struct cfg_node *next_BB = cfg_find_node_from_entry (next_bb_entry, root);
  if (next_BB == NULL) {
    fprintf (stderr, "** Find CFG Node Error **\n0x%jx: 0x%jx is not found.\n", bb_exit, next_bb_entry);
    exit (1);
  }

  path.push_back (next_BB);
*/
}

/* 
 * 
 */
static void
call_record (ADDRINT caller_addr, ADDRINT callee_addr, ADDRINT return_addr)
{
  call_stack.push_back (return_addr);
  branch_record (caller_addr, callee_addr);
}

static void
return_record (ADDRINT exit_addr, ADDRINT target_addr, CONTEXT *context)
{
  ADDRINT return_addr = call_stack.back ();
  if (target_addr != return_addr) {
    fprintf (stderr, "** call/return matching Error **\n0x%jx: return to 0x%jx but 0x%jx\n",
               exit_addr, target_addr, return_addr);
    exit (1);
  }
  call_stack.pop_back ();
}

/*****************************************************************************
 *                         Instrumentation functions                         *
 *****************************************************************************/

static void
instrument_branch (INS ins, void *v)
{
  if (start_addr == (uint64_t)INS_Address (ins))
    record_flag = true;
  else if (end_addr == (uint64_t)INS_Address (ins))
    record_flag = false;

  if (!record_flag) return;
  if (!INS_IsBranch (ins)) return;

  INS_InsertPredicatedCall(
    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)branch_record,
    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
    IARG_END
  );
  if (INS_HasFallThrough (ins)) {
    INS_InsertPredicatedCall(
      ins, IPOINT_AFTER, (AFUNPTR)branch_record, 
      IARG_INST_PTR, IARG_FALLTHROUGH_ADDR,
      IARG_END
    );
  }
}

static void
instrument_call (INS ins, void *v)
{
  if (!record_flag) return;
  if (!INS_IsCall (ins)) return;

  //IMG img = IMG_FindByAddress (INS_Address (ins));
  //if (!IMG_Valid (img) || !IMG_IsMainExecutable (img)) return;

  INS_InsertPredicatedCall(
    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)call_record,
    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
    IARG_RETURN_IP,
    IARG_END
  );
}

static void
instrument_return (INS ins, void *v)
{
  if (!record_flag) return;
  if (!INS_IsRet (ins)) return;

  INS_InsertPredicatedCall(
    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)call_record,
    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
    IARG_CONTEXT,
    IARG_END
  );
  if (INS_HasFallThrough (ins)) {
    INS_InsertPredicatedCall(
      ins, IPOINT_AFTER, (AFUNPTR)return_record,
      IARG_INST_PTR, IARG_ADDRINT, 0,
      IARG_CONTEXT,
      IARG_END
    );
  }
}

static void
instrument_syscall (INS ins, void *v)
{
  if (!record_flag) return;
  if (!INS_IsSyscall (ins)) return;
}

/*****************************************************************************
 *                               Other functions                             *
 *****************************************************************************/

static void
print_path ()
{
}

static void
fini(INT32 code, void *v)
{
  print_path ();
}


int
main(int argc, char *argv[])
{
  //for (int i = 0; i < argc; i++) {
  //  fprintf (stderr, "argv [%d]: %s\n", i, argv [i]);
  //}

  if (argc < 10) {
    fprintf(stderr, "usage: /home/binary/pin/pin-3.6-97554-g31f0a167d-gcc-linux/pin -t /home/binary/code/PathArmor/obj/kernel_module.so -- \n"
                   "         <test_file> <test_arg> <start_addr> <end_addr> <cfg_file>\n");
    exit (1);
  }

  PIN_InitSymbols();
  if(PIN_Init(argc, argv) != 0) {
    fprintf(stderr, "PIN_Init failed\n");
    return 1;
  }

  start_addr = (uint64_t) strtoul (argv [8], NULL, 0);
  end_addr   = (uint64_t) strtoul (argv [9], NULL, 0);

  cfg_load (argv [10]);
  cfg_print ();

  INS_AddInstrumentFunction(instrument_branch, NULL);
  INS_AddInstrumentFunction(instrument_call, NULL);
  INS_AddInstrumentFunction(instrument_return, NULL);
  INS_AddInstrumentFunction(instrument_syscall, NULL);
  PIN_AddFiniFunction(fini, NULL);

  PIN_StartProgram();
  
  return 1;
}

