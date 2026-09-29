#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "vm.h"
#include "net_host.h"
#include <net/if.h>

typedef uint8_t u8t;

int main() {
    char tun_name[IFNAMSIZ];
    int tun_fd = mac_utun_alloc(tun_name, sizeof(tun_name));
    if (tun_fd < 0) {
        fprintf(stderr, "Failed to allocate utun interface. (Run with sudo)\n");
        return 1;
    }

    configure_mac_tun(tun_name, "10.8.0.1", "10.8.0.2");
    make_nonblocking(tun_fd);
    
    char route_cmd[256];
    snprintf(route_cmd, sizeof(route_cmd), "sudo route add -net 10.8.0.2 -interface %s", tun_name);
    system(route_cmd);

    // Continuous Polling Bytecode Loop:
    // 0: Push Syscall 3 (Poll TUN)
    // 5: SYSCALL
    // 6: Push Syscall 1 (Print)
    // 11: SYSCALL
    // 12: JMP back to 0
    u8t bytecode[] = {
        // --- PC 0: Poll Network ---
        OP_CONST, 3, 0, 0, 0,           // 0: Push Syscall ID 3 (Poll)
        OP_SYSCALL,                     // 5: Polls packet, pushes length (or 0) to stack

        // --- PC 6: Check if length == 0 ---
        // If stack top is 0 (no packet), jump past the write block to PC 27
        OP_JZ, 27, 0, 0, 0,             // 6: If 0, skip to loop restart (PC 27)

        // --- PC 11: Write Packet Back (Echo) ---
        OP_CONST, 0xE8, 0x03, 0x00, 0x00,        // 11: Push source memory address 1000
        OP_CONST, 84, 0, 0, 0,          // 16: Push packet length
        OP_CONST, 4, 0, 0, 0,           // 21: Push Syscall ID 4 (Write)
        OP_SYSCALL,                     // 26: Write packet back to TUN!

        // --- PC 27: Loop Restart ---
        OP_JMP, 0, 0, 0, 0              // 27: Unconditional jump back to PC 0
    };

    VM vm;
    vm_init(&vm, bytecode, sizeof(bytecode), tun_fd);

    printf("Booting Network-Aware VM Runtime... Listening on %s\n", tun_name);
    vm_run(&vm);
    printf("VM shut down safely.\n");

    return 0;
}