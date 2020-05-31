#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "binary_loader/loader.h"
#include "triton_util/triton_util.h"
#include "disasm_util/disasm_util.h"
#include "cfg/cfg.h"

#include <string>
#include <map>
#include <vector>

#include <triton/api.hpp>
#include <triton/x86Specifications.hpp>

int  cfg_generation_init (void);
int  cfg_generation (char *bin_file, char *config_file, uint64_t entry, uint64_t exit);
int  cfg_generation_fini (void);
//int  cfg_save (char *cfg_file);
