#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_NATIVE__) || defined(__ISA_X86_64__) || defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_MIPS32__)
#define EXPECT_TYPE EM_MIPS
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
#define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_386
#else
#define EXPECT_TYPE -1
#endif

extern uint8_t ramdisk_start;
size_t ramdisk_read(void *buf, size_t offset, size_t len);


static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr *elf = (Elf_Ehdr *)(&ramdisk_start);
  assert(memcmp(elf->e_ident, ELFMAG, 4) == 0 && elf->e_machine == EXPECT_TYPE);
  Elf_Phdr *ph, *eph;
  ph = (void *)elf + elf->e_phoff;
  eph = ph + elf->e_phnum;
  while (ph < eph) {
    if (ph->p_type == PT_LOAD) {
      ramdisk_read((void *)ph->p_vaddr, ph->p_offset, ph->p_filesz);
      memset((void *)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }
    ph++;
  }
  return elf->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %#p", entry);
  ((void(*)())entry) ();
}

