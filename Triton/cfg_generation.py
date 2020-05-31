#!/usr/bin/env python2
## -*- coding: utf-8 -*-

import sys
import triton
import pintool

Triton = pintool.getTritonContext ()

def symbolize_inputs (tid):
  rdi = pintool.getCurrentRegisterValue (Triton.registers.rdi)	# argc
  rsi = pintool.getCurrentRegisterValue (Triton.registers.rsi)	# argv

  # for each string in argv
  while rdi > 1:
    addr = pintool.getCurrentMemoryValue (
               rsi + ((rdi-1) * triton.CPUSIZE.QWORD),
               triton.CPUSIZE.QWORD
             )
    # symbolize current argument string (including terminating NULL)
    c = None
    s = ''
    while c != 0:
      c = pintool.getCurrentMemoryValue (addr)
      s += chr(c)
      Triton.setConcreteMemoryToSymbolizeVariable (
          triton.MemoryAccess (addr, triton.CPUSIZE.BYTE)
        ).setComment ('argv[%d][%d]' % (rdi-1, len(s)-1))
      addr += 1
    rdi -=1
    print 'Symbolized argument %d: %s' % (rdi, s)


def main ():
  Triton.setArchitecture (triton.ARCH.X86_64)
  Triton.enableMode (triton.MODE.ALIGNED_MEMORY, True)

  pintool.startAnalysisFromSymbol ('main')

  pintool.insertCall (symbolize_inputs, pintool.INSERT_POINT.ROUTINE_ENTRY, 'main')

  pintool.runProgram ()


if __name__ == '__main__':
  main ()
