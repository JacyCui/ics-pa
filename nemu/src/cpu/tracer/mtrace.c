#include <common.h>

#ifdef CONFIG_MTRACE

typedef struct MTrace {
    paddr_t paddr;
    vaddr_t pc;
    int type;
    struct MTrace *next;
} MTrace;

enum {PREAD, PWRITE};

static MTrace *head = NULL, *tail = NULL;

void mtrace_add(paddr_t paddr, vaddr_t pc, int type) {
    if (paddr == pc) {
        return;
    }
    MTrace *p = (MTrace *)malloc(sizeof(MTrace));
    p->paddr = paddr;
    p->pc = pc;
    p->type = type;
    p->next = NULL;
    if (head == NULL) {
        head = p;
        tail = p;
        return;
    }
    tail->next = p;
    tail = tail->next;
}

void mtrace_display() {
    MTrace *p = head;
    printf("%sMemory trace:%s\n", ANSI_FG_YELLOW, ANSI_NONE);
    while (p != NULL) {
        if (p->type == PREAD) {
            printf("Read from physical address %#08x when pc = %#08x.\n", p->paddr, p->pc);
        }
        if (p->type == PWRITE) {
            printf("Write to physical address %#08x when pc = %#08x.\n", p->paddr, p->pc);
        }
        p = p->next;
    }
}

void mtrace_clear() {
    Log("Clearing memory trace buffer ...");
    MTrace *p = head;
    MTrace *tmp;
    while (p != NULL) {
        tmp = p;
        p = p->next;
        free(tmp);
    }
    head = NULL;
    tail = NULL;
}

#endif
