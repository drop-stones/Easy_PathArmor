#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <deque>
#include <string>
#include <algorithm>

#include "pin.H"
#include "/home/binary/code/PathArmor/cfg_generation/src/cfg.hpp"

static uint64_t start_addr, end_addr;
static bool instrument_flag = false;
static std::deque <uint64_t> path;

static bool
check_flag (bool old_bool, uint64_t addr)
{
  if (addr == start_addr) {
    path.push_back (start_addr);
    return true;
  } else if (addr == end_addr) {
    path.push_back (end_addr);
    return false;
  } else
    return old_bool;
}

static bool
check_text_section (INS ins)
{
  RTN rtn = INS_Rtn (ins);
  SEC sec = RTN_Sec (rtn);
  IMG img = SEC_Img (sec);

  if (!IMG_Valid (img) || !IMG_IsMainExecutable (img))
    return false;
  if (!SEC_Valid (sec) || strncmp (SEC_Name (sec).c_str (), ".text", 5))
    return false;
  if (INS_IsDirectBranchOrCall (ins)) {
    /* Only direct branch or call */
    ADDRINT target_addr = INS_DirectBranchOrCallTargetAddress (ins);
    rtn = RTN_FindByAddress (target_addr);
    sec = RTN_Sec (rtn);
    img = SEC_Img (sec);
    if (!IMG_Valid (img) || !IMG_IsMainExecutable (img))
      return false;
    if (!SEC_Valid (sec) || strncmp (SEC_Name (sec).c_str (), ".text", 5))
      return false;
  }
  return true;
}

/*****************************************************************************
 *                             Analysis functions                            *
 *****************************************************************************/

static void
branch_record (ADDRINT exit, ADDRINT next_entry)
{
  path.push_back (exit);
  path.push_back (next_entry);
}

static void
call_record (ADDRINT caller_addr, ADDRINT callee_addr, ADDRINT return_addr)
{
  path.push_back (caller_addr);
  path.push_back (callee_addr);
}

static void
return_record (ADDRINT exit_addr, ADDRINT target_addr)
{
  path.push_back (exit_addr);
  path.push_back (target_addr);
}

static void
path_verification (ADDRINT syscall_addr, ADDRINT syscall_number)
{
  if (cfg_check_validity (path))
    fprintf (stderr, "valid\n");
  else
    fprintf (stderr, "invalid\n");
}

/*****************************************************************************
 *                         Instrumentation functions                         *
 *****************************************************************************/

static void
instrument_branch (INS ins, void *v)
{
  if (!INS_IsBranch (ins)) return;

  INS_InsertPredicatedCall (
    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) branch_record,
    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
    IARG_END
  );
  if (INS_HasFallThrough (ins)) {
    INS_InsertPredicatedCall (
      ins, IPOINT_AFTER, (AFUNPTR) branch_record, 
      IARG_INST_PTR, IARG_FALLTHROUGH_ADDR,
      IARG_END
    );
  }
}

static void
instrument_call (INS ins, void *v)
{
  if (!INS_IsCall (ins)) return;

  INS_InsertPredicatedCall (
    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) call_record,
    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
    IARG_RETURN_IP,
    IARG_END
  );
}

static void
instrument_return (INS ins, void *v)
{
  if (!INS_IsRet (ins)) return;

  INS_InsertPredicatedCall (
    ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) call_record,
    IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
    IARG_END
  );
}

static void
instrument_syscall (INS ins, void *v)
{
  if (!INS_IsSyscall (ins)) return;

  INS_InsertPredicatedCall (
    ins, IPOINT_BEFORE, (AFUNPTR) path_verification,
    IARG_INST_PTR, IARG_SYSCALL_NUMBER,
    IARG_END
  );
}

static void
instrument_ins (INS ins, void *v)
{
  if (!(instrument_flag = check_flag (instrument_flag, (uint64_t)INS_Address (ins))))
    return;
  if (!check_text_section (ins))
    return;

  instrument_branch  (ins, NULL);
  instrument_call    (ins, NULL);
  instrument_return  (ins, NULL);
  instrument_syscall (ins, NULL);
}

static void
instrument_rtn (RTN rtn, void *v)
{
  if (!RTN_Valid (rtn)) return;
  SEC sec = RTN_Sec (rtn);
  if (!SEC_Valid (sec)) return;
  IMG img = SEC_Img (sec);
  if (!IMG_Valid (img) || !IMG_IsMainExecutable (img)) return;
  if (strncmp (SEC_Name (sec).c_str (), ".text", 5) != 0) return;

  fprintf (stderr, "sec = %s\n", SEC_Name (sec).c_str ());

  RTN_Open (rtn);
  for (INS ins = RTN_InsHead (rtn); INS_Valid (ins); ins = INS_Next (ins)) {
    /* instrument INS */
    uint64_t ins_addr = (uint64_t) INS_Address (ins);
    if (start_addr <= ins_addr && ins_addr <= end_addr) {
      instrument_branch  (ins, NULL);
      instrument_call    (ins, NULL);
      instrument_return  (ins, NULL);
      instrument_syscall (ins, NULL);
    }
  }
  RTN_Close (rtn);
}

/*****************************************************************************
 *                               Other functions                             *
 *****************************************************************************/

static void
print_path ()
{
  while (path.size () != 1) {
   printf ("0x%jx -> ", path.front ());
   path.pop_front ();
  }
  printf ("0x%jx\n", path.front ());
}

static void
fini(INT32 code, void *v)
{
  //print_path ();
  path_verification (0, 0);
}

int
main(int argc, char *argv[])
{
  if (argc < 10) {
    fprintf(stderr, "usage: /home/binary/pin/pin-3.6-97554-g31f0a167d-gcc-linux/pin -t /home/binary/code/PathArmor/obj/kernel_module.so -- \n"
                    "         <test_file> <test_arg1> ... <start_addr> <end_addr> <cfg_file>\n");
    exit (1);
  }

  PIN_InitSymbols();
  if(PIN_Init(argc, argv) != 0) {
    fprintf(stderr, "PIN_Init failed\n");
    return 1;
  }

  start_addr = (uint64_t) strtoul (argv [argc-3], NULL, 0);
  end_addr   = (uint64_t) strtoul (argv [argc-2], NULL, 0);

  cfg_load (argv [argc-1]);

  INS_AddInstrumentFunction (instrument_ins, NULL);
  PIN_AddFiniFunction(fini, NULL);

  PIN_StartProgram();
 
  return 1;
}
