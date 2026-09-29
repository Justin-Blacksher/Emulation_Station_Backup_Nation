#ifndef VM_H
#define VM_H

#include <stdint.h>

#define MEMORY_SIZE (64 * 1024)
#define STACK_SIZE 256

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    int pc;
    int sp;
    int stack[STACK_SIZE];
    int running;
    int tun_fd;
} VM;

typedef enum {
    OP_HALT = 0,
    OP_CONST,
    OP_LOAD,
    OP_STORE,
    OP_ADD,
    OP_SUB,
    OP_JZ,
    OP_JMP,
    OP_SYSCALL
} Opcode;

void vm_init(VM *vm, uint8_t *bytecode, int size, int tun_fd);
void vm_run(VM *vm);

#endif