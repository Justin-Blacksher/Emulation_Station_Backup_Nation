# Network-Aware Virtual Machine

A small C-based stack virtual machine prototype for macOS. It allocates a
kernel `utun` interface and runs bytecode that can poll the interface and write
packet data back to it.

## Requirements

- macOS (the host networking code uses Apple's `utun` and `PF_SYSTEM` APIs)
- Xcode Command Line Tools, including a C compiler
- Administrator privileges to create/configure the tunnel and add its route

## Build and run

From the project directory:

```sh
gcc main.c vm.c net_host.c -o vpn_vm
sudo ./vpn_vm
```

The program configures a tunnel with local address `10.8.0.1` and remote
address `10.8.0.2`, then enters the VM's polling loop. Stop it with
**Ctrl-C**.

## Project layout

- `main.c` allocates and configures the tunnel, defines the program's bytecode,
  and starts the VM.
- `vm.c` implements the stack, instruction dispatch, and network-related
  system calls.
- `vm.h` defines the VM state and instruction opcodes.
- `net_host.c` and `net_host.h` implement macOS tunnel allocation,
  configuration, and nonblocking mode.

## VM instructions

The VM has 64 KiB of memory and a 256-entry integer stack. The bytecode
currently supports:

| Instruction | Behavior |
| --- | --- |
| `OP_HALT` | Stop execution |
| `OP_CONST` | Push a 32-bit constant |
| `OP_LOAD` | Replace a stack address with the 32-bit value at that address |
| `OP_STORE` | Store a stack value at a stack-provided address |
| `OP_ADD`, `OP_SUB` | Pop two values and push their sum or difference |
| `OP_JZ` | Pop a value and jump if it is zero |
| `OP_JMP` | Jump to an absolute bytecode address |
| `OP_SYSCALL` | Dispatch the syscall ID at the top of the stack |

The implemented syscall IDs are:

| ID | Behavior |
| --- | --- |
| `1` | Print a positive integer value from the stack |
| `3` | Read a packet from the tunnel and copy its payload to VM memory at
  address `1000`; push the payload length, or zero when no packet is read |
| `4` | Write packet data from VM memory to the tunnel; expects the length and
  source address below the syscall ID on the stack |

## Current behavior and limitations

The bytecode in `main.c` polls the tunnel and, when a packet is available,
writes data from VM memory back to the interface. It currently uses a fixed
write length of 84 bytes rather than the actual length returned by the read
syscall. The program is an experimental prototype, not a production VPN or
general-purpose VM.

The VM does not validate bytecode sizes, memory addresses, stack depth, or
instruction targets. Only run bytecode you trust. Tunnel setup and route
configuration also depend on macOS system APIs and privileged networking
operations.
