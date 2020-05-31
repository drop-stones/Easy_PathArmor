#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "pin.H"
#include "../../Triton/disasm_util/disasm_util.h"
//#include "../Triton/cfg_generation.h"

struct basic_block {
  basic_block ()                                             : entry(0),     exit(0),    id(0)      {}
  basic_block (ADDRINT entry, ADDRINT exit, unsigned int id) : entry(entry), exit(exit), id(id)     {}
  ADDRINT       entry;
  ADDRINT       exit;
  unsigned int  id;

  bool
  in_basic_block (ADDRINT addr) {
    if (exit == 0)
      return entry <= addr;
    return entry <= addr && addr <= exit;
  }
};

struct call_block {
  call_block () : caller_addr(0), return_addr(0), entry(0), exit(0), id(0) {}
  call_block (ADDRINT caller_addr, ADDRINT return_addr, ADDRINT entry, ADDRINT exit, unsigned int id)
                : caller_addr(caller_addr), return_addr(return_addr), entry(entry), exit(exit), id(id) {}
  ADDRINT       caller_addr;
  ADDRINT       return_addr;
  ADDRINT       entry;
  ADDRINT       exit;
  unsigned int  id;

  bool
  in_call_block (ADDRINT addr) {
    if (exit == 0)
      return entry <= addr;
    return entry <= addr && addr <= exit;
  }
};

// FILE *logfile;
std::vector<struct basic_block> path;
std::map<unsigned int, std::map<unsigned int, struct call_block> > calls;

/*****************************************************************************
 *                             Analysis functions                            *
 *****************************************************************************/

static int
return_basic_block_id (ADDRINT addr)
{
  unsigned int i;
  struct basic_block bb;
  for (i = 0; i < path.size (); i++) {
    bb = path [i];
    if (bb.in_basic_block (addr))
      return bb.id;
  }

  return -1;
}

static int
return_call_block_id (unsigned int bb_id, ADDRINT addr)
{
  std::map<unsigned int, struct call_block> call_blocks = calls [bb_id];
  std::map<unsigned int, struct call_block>::iterator ite;
  struct call_block cb;
  for (ite = call_blocks.begin (); ite != call_blocks.end (); ite++) {
    cb = ite->second;
    if (cb.in_call_block (addr))
      return cb.id;
  }

  return -1;
}

/* 
 * 
 */
static void
branch_record (ADDRINT bb_exit, ADDRINT next_bb_entry)
{
  PIN_LockClient ();

  IMG now_img, next_img;
  now_img  = IMG_FindByAddress (bb_exit);
  next_img = IMG_FindByAddress (next_bb_entry);
  if (!IMG_Valid (now_img) || !IMG_Valid (next_img))
    return;
  if (!IMG_IsMainExecutable (now_img) && !IMG_IsMainExecutable (next_img))
    return;

  static unsigned int bb_id = 0;
  if (IMG_IsMainExecutable (now_img)) {
    if (path.size () != 0) {
      struct basic_block &back_bb = path.back ();
      back_bb.exit = bb_exit;
    }
  }

  if (IMG_IsMainExecutable (next_img)) {
    struct basic_block new_bb (next_bb_entry, 0, bb_id);
    path.push_back (new_bb);
    bb_id++;
  }

  PIN_UnlockClient ();
}

/* 
 * 
 */
static void
call_record (ADDRINT caller_addr, ADDRINT callee_addr, ADDRINT return_addr)
{
  static unsigned int cb_id = 0;
  int bb_id = return_basic_block_id (caller_addr);
  if (bb_id < 0) return;

  struct call_block new_cb (caller_addr, return_addr, callee_addr, 0, cb_id);
  calls [(unsigned int)bb_id] [cb_id] = new_cb;
  cb_id++;
}

static void
return_record (ADDRINT exit_addr, ADDRINT target_addr, CONTEXT *context)
{
  ADDRINT return_addr;

  if (target_addr == 0) {
    return_addr = 0;
    UINT8 *ptr = (UINT8*) &return_addr;
    PIN_GetContextRegval (context, REG_INST_PTR, ptr);
    if (return_addr == 0) return;
  } else {
    return_addr = target_addr;
  }

  int bb_id = return_basic_block_id (return_addr);
  int cb_id = return_call_block_id  (bb_id, return_addr);
  if (bb_id < 0 || cb_id < 0) return;
  struct call_block &cb = calls [(unsigned int)bb_id] [(unsigned int)cb_id];

  //if (cb.return_addr != return_addr)  return;

  cb.exit = exit_addr;
}

/*****************************************************************************
 *                         Instrumentation functions                         *
 *****************************************************************************/
static void
instrument_branch (INS ins, void *v)
{
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
  if (!INS_IsCall (ins)) return;

  IMG img = IMG_FindByAddress (INS_Address (ins));
  if (!IMG_Valid (img) || !IMG_IsMainExecutable (img)) return;

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

}
/*****************************************************************************
 *                               Other functions                             *
 *****************************************************************************/

static void
print_path ()
{
  unsigned int i;
  std::map<unsigned int, std::map<unsigned int, struct call_block> >::iterator calls_ite;
  std::map<unsigned int, struct call_block>::iterator call_blocks_ite;

  printf ("Basic Block information: \n");
  for (i = 0; i < path.size (); i++) {
      struct basic_block bb = path [i];
      printf ("    BB%02u : 0x%jx - 0x%jx\n", bb.id, bb.entry, bb.exit);
  }

  printf ("Call Block information: \n");
  for (calls_ite = calls.begin (); calls_ite != calls.end (); calls_ite++) {
    std::map<unsigned int, struct call_block> call_blocks = calls_ite->second;
    for (call_blocks_ite = call_blocks.begin (); call_blocks_ite != call_blocks.end (); call_blocks_ite++) {
      struct call_block cb = call_blocks_ite->second;
      printf ("    CB%02u : call@0x%jx -> entry@0x%jx -> exit@0x%jx -> return@0x%jx \n", 
                        cb.id, cb.caller_addr, cb.entry, cb.exit, cb.return_addr);
    }
  }

  printf ("Path information: \n  ");
  for (i = 0; i < path.size (); i++) {
    struct basic_block bb = path [i];
    printf ("BB%02u -> ", bb.id);
  }
  printf ("<fini>\n");
  fflush (stdout);
}

static void
fini(INT32 code, void *v)
{
  print_path ();
}


int
main(int argc, char *argv[])
{
  fprintf (stderr, "start!\n");
  for (int i = 0; i < argc; i++) {
    fprintf (stderr, "argv [%d]: %s\n", i, argv [i]);
  }

  // PIN_InitSymbols();
  if(PIN_Init(argc, argv) != 0) {
    fprintf(stderr, "PIN_Init failed\n");
    return 1;
  }


  //uint64_t start = strtoul (argv [9], NULL, 0);
  //uint64_t end   = strtoul (argv [10], NULL, 0);

  //cfg_generation_init ();
  //cfg_generation_config (argv [6], argv [8]);
  //cfg_gen (start, end);
  //cfg_print ();
  //cfg_generation_fini (); 
  //fflush (stdout);

  INS_AddInstrumentFunction(instrument_branch, NULL);
  INS_AddInstrumentFunction(instrument_call, NULL);
  INS_AddInstrumentFunction(instrument_return, NULL);
  INS_AddInstrumentFunction(instrument_syscall, NULL);
  PIN_AddFiniFunction(fini, NULL);

  PIN_StartProgram();
    
  return 1;
}

