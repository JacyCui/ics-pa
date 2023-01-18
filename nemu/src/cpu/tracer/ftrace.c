#include <common.h>
#include <cpu/decode.h>

#ifdef CONFIG_FTRACE

#define CALL_STACK_MAXLEN 256

static struct ftrace {
  word_t pc;
  word_t value;
  char symbol[256];
} func_stack[CALL_STACK_MAXLEN];

static int top = 0;

void ftrace_add(Decode *d) {
  if (top >= CALL_STACK_MAXLEN) {
    printf("%s\n", ANSI_FMT("Ftrace overflow!", ANSI_FG_RED));
    return;
  }
  //ret
  if (d->isa.inst.val == 0x00008067 && top > 0) {
    top--;
    return;
  }
  bool success = true;
  const char *func = isa_lookup_symtab_by_address(d->dnpc, &success);
  if (!success) {
    return;
  }
  func_stack[top].pc = d->pc;
  func_stack[top].value = d->dnpc;
  strcpy(func_stack[top].symbol, func);
  top++;
}

void ftrace_display() {
  if (isa_symtab_size() == 0) {
    printf("%sUnable to display call stack due to lack of a symbol table.%s\n", ANSI_FG_RED, ANSI_NONE);
    return;
  }
  if (top == 0) {
    printf("%sEmpty call stack.%s\n", ANSI_FG_YELLOW, ANSI_NONE);
    return;
  }
  int i;
  printf("%sCall Stack: %d%s\n", ANSI_FG_YELLOW, top, ANSI_NONE);
  printf("------------------------------------\n");
  for (i = 0; i < top && i < CALL_STACK_MAXLEN; i++) {
    printf("[%d] Call function %s(%#08x) at pc = %#08x\n", i, func_stack[i].symbol, func_stack[i].value, func_stack[i].pc);
  }
  printf("------------------------------------\n");
}

void ftrace_clear() {
  Log("Clearing function call trace buffer ...");
  top = 0;
}

#endif
