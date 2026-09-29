#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE (64 * 1024)
#define STACK_SIZE 256

typedef struct 
{
    uint8_t memory[MEMORY_SIZE];    // Flat Memory Space
    int pc;                         // Program Counter
    int sp;                         // Stack Pointer
    int stack[STACK_SIZE];          // Operand stack for calculations
    int running;                    // Flag to keep the VM loop alive
} VM;

typedef enum 
{
    OP_HALT = 0,                    // Stop the VM
    OP_CONST,                       // Push a constant integer onto the stack
    OP_LOAD,                        // Load 4 bytes from memory address at stack top
    OP_STORE,                       // Store 4 bytes from stack into memory address
    OP_ADD,                         // Pop two numbers, add them, push result
    OP_SUB,                         // Pop two numbers, subtract, push result
    OP_JZ,                          // Jump if zero (conditional branch)
    OP_JMP,                         // Unconditional jump
    OP_SYSCALL                      // Trigger a host system call (e.g., render, sleep, print)
} Opcode;

void vm_init(VM *vm, uint8_t *bytecode, int size)
{
    memset(vm, 0, sizeof(VM));
    // Load bytecode into the beginning of VM memory
    memcpy(vm->memory, bytecode, size);
    vm->pc = 0;
    vm->sp = -1;
    vm->running = 1;
}

void vm_run(VM *vm)
{
    while (vm->running)
    {
        // Fetch Opcode
        uint8_t opcode = vm->memory[vm->pc++];

        switch (opcode)
        {
            case OP_HALT: 
            {
                vm->running = 0;
                break;
            }

            case OP_CONST: 
            {
                // Read a 4-byte integer from the instruction stream
                int value = *(int*)&vm->memory[vm->pc];
                vm->pc += 4;
                vm->stack[++vm->sp] = value;
                break;
            }

            case OP_LOAD:
            {
                int address = vm->stack[vm->sp--];
                int value = *(int*)&vm->memory[address];
                vm->stack[++vm->sp] = value;
                break;
            }

            case OP_STORE:
            {
                int value = vm->stack[vm->sp--];
                int address = vm->stack[vm->sp--];
                *(int*)&vm->memory[address] = value;
                break;
            }

            case OP_ADD:
            {
                int b = vm->stack[vm->sp--];
                int a = vm->stack[vm->sp--];
                vm->stack[++vm->sp] = a + b;
                break;
            }

            case OP_SUB:
            {
                int b = vm->stack[vm->sp--];
                int a = vm->stack[vm->sp--];
                vm->stack[++vm->sp] = a - b;
                break;
            }

            case OP_SYSCALL:
            {
                int syscall_num = vm->stack[vm->sp--];
                
                if (syscall_num == 1) 
                {
                    // Syscall 1: Print stat value from top of stack
                    int val = vm->stack[vm->sp--];
                    printf("[Tamagotchi Status Print] Value: %d\n", val);
                }
                else if (syscall_num == 2)
                {
                    // syscall 2: Get user input from the C host
                    int input;
                    printf("\n ----- TAMAGOTCHI ACTION MENU -----\n");
                    printf("1. Feed Your Pet (Decrease Hunger)\n");
                    printf("2. Do Nothing\n");
                    printf("Enter Choice: ");
                    scanf("%d", &input);

                    vm->stack[++vm->sp]= input;
                }
                break;
            }

            case OP_JMP:
            {
                // Read the target PC address from the instruction stream
                int target_pc = *(int*)&vm->memory[vm->pc];
                // Force the program counter to the target address
                vm->pc = target_pc;
                break;
            }

            case OP_JZ:
            {
                // Read the target PC address from the instruction stream
                int target_pc = *(int*)&vm->memory[vm->pc];
                vm->pc += 4;    // Skip past the target address 

                // Pop condition from the stack
                int condition = vm->stack[vm->sp--];

                // If the condition is zero, jump!
                if (condition == 0)
                {
                    vm->pc = target_pc;
                }
                break;

            }

            default:
            {
                printf("Unknown opcode: 0x%02X at PC=%d\n", opcode, vm->pc - 1);
                vm->running = 0;
                break;
            }
        }
    }
}

int main()
{
    /*
    1. Stores a starting hunger value of 50 into memory address 0
    2. Loads hunger from address 0
    3. pushes 10 (Feeding the pet)
    4. Adds them together
    5. Stores the new value back to address 0.
    6. Loads address 0 and triggers SYSCALL 1 to print it
    7. Halts
    */


    uint8_t bytecode[] = {
        // --- 0:9 initializztion
        OP_CONST, 0, 0, 0, 0,
        OP_CONST, 50, 0, 0, 0,
        OP_STORE,                   

        OP_CONST, 0, 0, 0, 0,
        OP_LOAD,
        OP_CONST, 1, 0, 0, 0,
        OP_SYSCALL,

        OP_CONST, 2, 0, 0, 0,
        OP_SYSCALL,

        OP_CONST, 0, 0, 0, 0,
        OP_CONST, 0, 0, 0, 0, 
        OP_LOAD,
        OP_CONST, 10, 0, 0, 0,
        OP_SUB,
        OP_STORE,

        OP_JMP, 11, 0, 0, 0,

        OP_HALT
    };

    VM vm;
    vm_init(&vm, bytecode, sizeof(bytecode));

    printf("Booting Tamagotchi VM....\n");
    vm_run(&vm);
    printf("Tamagotchi VM shut down safely.\n");
    return 0;
}