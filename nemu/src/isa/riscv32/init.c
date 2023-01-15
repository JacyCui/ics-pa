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

#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x800002b7,  // lui t0,0x80000
  0x0002a023,  // sw  zero,0(t0)
  0x0002a503,  // lw  a0,0(t0)
  0x00100073,  // ebreak (used as nemu_trap)
};

static void restart() {
  /* Set the initial program counter. */
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  cpu.gpr[0] = 0;
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}

#define SYMBOL_MAX_NUM 128

static struct symbol_entry {
  char symbol[128];
  char type[8];
  word_t value;
} symbol_table[SYMBOL_MAX_NUM] = {0};

static size_t symbol_num = 0;

void isa_load_symtab(const char *elf_file) {
  if (elf_file == NULL) {
    Log("No elf-file is given, won't load symbol table.");
    return;
  }
  Log("Loading elf file %s ...", elf_file);
  FILE *fp = fopen(elf_file, "rb");
  if (fp == NULL) {
    Log("%sFail to open elf-file %s, won't load symbol table.%s", ANSI_FG_YELLOW, elf_file, ANSI_NONE);
    return;
  }

  // Read elf header
  Elf32_Ehdr *elf_head = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  int ret = fread(elf_head, sizeof(Elf32_Ehdr), 1, fp);
  assert(ret == 1);

  // Read section header
  Elf32_Shdr *shdr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * elf_head->e_shnum);
  fseek(fp, elf_head->e_shoff, SEEK_SET);
  ret = fread(shdr, sizeof(Elf32_Shdr) * elf_head->e_shnum, 1, fp);
  assert(ret == 1);

  // Read string table and symbol table
  size_t i;
  Elf32_Word symtab_offset = 0, symtab_size = 0;
  Elf32_Word strtab_offset = 0, strtab_size = 0;
  for (i = 0; i < elf_head->e_shnum; i++) {
    if (shdr[i].sh_type == SHT_SYMTAB) {
      symtab_offset = shdr[i].sh_offset;
      symtab_size = shdr[i].sh_size;
    }
    if (shdr[i].sh_type == SHT_STRTAB && i != elf_head->e_shstrndx) {
      strtab_offset = shdr[i].sh_offset;
      strtab_size = shdr[i].sh_size;
    }
  }

  Elf32_Sym *symtab = (Elf32_Sym *)malloc(symtab_size);
  fseek(fp, symtab_offset, SEEK_SET);
  ret = fread(symtab, symtab_size, 1, fp);
  assert(ret == 1);

  char *strtab = (char *)malloc(strtab_size);
  fseek(fp, strtab_offset, SEEK_SET);
  ret = fread(strtab, strtab_size, 1, fp);
  assert(ret == 1);

  // build symbol table for functions and global objects
  size_t sym_num = symtab_size / sizeof(Elf32_Sym);
  for (i = 0; i < sym_num; i++) {
    if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
      symbol_table[symbol_num].value = symtab[i].st_value;
      strcpy(symbol_table[symbol_num].symbol, strtab + symtab[i].st_name);
      strcpy(symbol_table[symbol_num].type, "FUNC");
      symbol_num++;
    }
    if (ELF32_ST_TYPE(symtab[i].st_info) == STT_OBJECT) {
      symbol_table[symbol_num].value = symtab[i].st_value;
      strcpy(symbol_table[symbol_num].symbol, strtab + symtab[i].st_name);
      strcpy(symbol_table[symbol_num].type, "OBJECT");
      symbol_num++;
    }
  }

  free(elf_head);
  free(shdr);
  free(symtab);
  free(strtab);
  fclose(fp);
}

size_t isa_symtab_size() {
  return symbol_num;
}

word_t isa_lookup_symtab_by_name(const char *symbol, bool *success) {
  size_t i;
  for (i = 0; i < symbol_num; i++) {
    if (strcmp(symbol_table[i].symbol, symbol) == 0) {
      return symbol_table[i].value;
    }
  }
  printf("Unknown symbol: %s\n", symbol);
  *success = false;
  return 0;
}

const char *isa_lookup_symtab_by_address(vaddr_t vaddr, bool *success) {
  size_t i;
  for (i = 0; i < symbol_num; i++) {
    if (symbol_table[i].value == vaddr) {
      return symbol_table[i].symbol;
    }
  }
  *success = false;
  return NULL;
}

void isa_display_symtab() {
  if (symbol_num == 0) {
    printf("No symbol table loaded!\n");
    return;
  }
  size_t i;
  printf("Symbol Table\n------------------------------------------------------------------------------------\n");
  printf("%-20s%-10s%-20s%-20s%-20s\n", "symbol", "type", "hexdecimal", "unsigned decimal", "signed decimal");
  printf("------------------------------------------------------------------------------------\n");
  for (i = 0; i < symbol_num; i++) {
    printf("%-20s%-10s%#-20x%-20u%-20d\n", symbol_table[i].symbol, symbol_table[i].type, symbol_table[i].value, symbol_table[i].value, symbol_table[i].value);
  }
  printf("------------------------------------------------------------------------------------\n");
}


