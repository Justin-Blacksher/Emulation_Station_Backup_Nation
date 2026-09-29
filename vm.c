#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "vm.h"

typedef uint8_t u8t;
typedef uint32_t u32t;

void vm_init(VM *vm, uint8_t *bytecode, int size, int tun_fd) {
    memset(vm, 0, sizeof(VM));
    memcpy(vm->memory, bytecode, size);
    vm->pc = 0;
    vm->sp = -1;
    vm->running = 1;
    vm->tun_fd = tun_fd;
}

void vm_run(VM *vm) {
    while (vm->running) {
        uint8_t opcode = vm->memory[vm->pc++];

        switch (opcode) {
            case OP_HALT:
                vm->running = 0;
                break;

            case OP_CONST: {
                int value = *(int*)&vm->memory[vm->pc];
                vm->pc += 4;
                vm->stack[++vm->sp] = value;
                break;
            }

            case OP_LOAD: {
                int address = vm->stack[vm->sp--];
                int value = *(int*)&vm->memory[address];
                vm->stack[++vm->sp] = value;
                break;
            }

            case OP_STORE: {
                int value = vm->stack[vm->sp--];
                int address = vm->stack[vm->sp--];
                *(int*)&vm->memory[address] = value;
                break;
            }

            case OP_ADD: {
                int b = vm->stack[vm->sp--];
                int a = vm->stack[vm->sp--];
                vm->stack[++vm->sp] = a + b;
                break;
            }

            case OP_SUB: {
                int b = vm->stack[vm->sp--];
                int a = vm->stack[vm->sp--];
                vm->stack[++vm->sp] = a - b;
                break;
            }

            case OP_JMP: {
                int target_pc = *(int*)&vm->memory[vm->pc];
                vm->pc = target_pc;
                break;
            }

            case OP_JZ: {
                int target_pc = *(int*)&vm->memory[vm->pc];
                vm->pc += 4;
                int condition = vm->stack[vm->sp--];
                if (condition == 0) {
                    vm->pc = target_pc;
                }
                break;
            }

            case OP_SYSCALL: {
                int syscall_num = vm->stack[vm->sp--];

                if (syscall_num == 1) {
                    int val = vm->stack[vm->sp--];
                    // Only print when a real packet arrives to keep console clean
                    if (val > 0) {
                        printf("[VM Status] Packet Payload Size: %d bytes\n", val);
                    }
                }
                else if (syscall_num == 3) {
                    uint8_t buf[2048];
                    int nread = read(vm->tun_fd, buf, sizeof(buf));
                    
                    if (nread > 4) {
                        // MacOS utun prepends 4-bytes
                        int packet_len = nread - 4;

                        // Copy into memory
                        memcpy(&vm->memory[1000], buf + 4, packet_len);
                        vm->stack[++vm->sp] = packet_len;
                    } else {
                        // Non-blocking read would block (EAGAIN/EWOULDBLOCK) or error
                        vm->stack[++vm->sp] = 0;     
                    }

                }
                else if (syscall_num == 4) {
                    // Syscall 4: Write packet back out to tun (Egress)
                    // Expects: stack has [length], [source_memory_address]
                    int length = vm->stack[vm->sp--];
                    int address = vm->stack[vm->sp--];
                    
                    if (length > 0)
                    {
                        u8t frame[2048];
                        u32t af_inet = AF_INET;

                        // Copy 4 byte header
                        memcpy(frame, &af_inet, 4);
                        // Copy actual packet data
                        memcpy(frame + 4, &vm->memory[address], length);

                        int written = write(vm->tun_fd, frame, length + 4);
                        if (written > 0)
                        {
                            printf("[Network Egress] Reflected Packet back (%d bytes written)\n", written - 4);
                        }
                    }
                }
                break;
            }

            default:
                printf("Unknown opcode: 0x%02X at PC=%d\n", opcode, vm->pc - 1);
                vm->running = 0;
                break;
        }
    }
}