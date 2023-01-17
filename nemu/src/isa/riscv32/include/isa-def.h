/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __ISA_RISCV32_H__
#define __ISA_RISCV32_H__

#include <common.h>

typedef struct {
  word_t gpr[32];
  vaddr_t pc;

  word_t mstatus;
  word_t mtvec;
  vaddr_t mepc;
  word_t mcause;

} riscv32_CPU_state;

typedef struct {
      word_t uie : 1;
      word_t sie : 1;
      word_t reserved_0 : 1;
      word_t mie : 1;
      word_t upie : 1;
      word_t spie : 1;
      word_t reserved_1 : 1;
      word_t mpie : 1;
      word_t spp : 1;
      word_t reserved_3_2 : 2;
      word_t mpp : 2;
      word_t fs : 2;
      word_t xs : 2;
      word_t mprv : 1;
      word_t sum : 1;
      word_t mxr : 1;
      word_t tvm : 1;
      word_t tw : 1;
      word_t tsr : 1;
      word_t reserved_11_4 : 8;
      word_t sd : 1;
} mstatus_t;

typedef struct {
  word_t mode : 2;
  word_t base : 30;
} mtvec_t;

typedef struct {
  word_t exception_code : 31;
  word_t interrupt : 1;
} mcause_t;


#define CSR_MASK 0xfff
#define CSR_MSTATUS_ADDR 0x300
#define CSR_MTVEC_ADDR 0x305
#define CSR_MEPC_ADDR 0x341
#define CSR_MCAUSE_ADDR 0x342

#define TRAP_MECALL 0xb


// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} riscv32_ISADecodeInfo;

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
