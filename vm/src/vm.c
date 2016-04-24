#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gc/gc.h>

#include "vm.h"
#include "opcodes.h"

vm_t *vm_new() {
    vm_t *vm;

    vm = malloc(sizeof(struct vm));
    if (!vm)
        return NULL;
    memset(vm, '\0', sizeof(struct vm));

    /**
     * Allocate 64k for the program.
     */
    vm->code = malloc(0xFFFF);
    if (vm->code == NULL) {
        return NULL;
    }

    vm->ip = 0;
    vm->current_frame = 0;
    vm->running = true;

    memset(vm->code, '\0', 0xFFFF);

    opcode_init(vm);

    return vm;
}


/**
 * The VM has a single array for code, but it also keeps track
 * of modules separately.
 */
void vm_load_module(vm_t *vm, unsigned char *code, unsigned int size) {
    memcpy(vm->code, code, size);
}

void vm_run(vm_t * vm) {
    GC_init();
    int iterations = 0;

    /**
     * Run continuously.
     *
     * In practice this means until an EXIT instruction is encountered,
     * which will set the "running"-flag to be false.
     *
     */
    while (vm->running == true) {
        /**
         * Lookup the instruction at the instruction-pointer.
         */
        int opcode = vm->code[vm->ip];
        /**
         * Call the opcode implementation, if defined.
         */
        if (vm->opcodes[opcode] != NULL)
            vm->opcodes[opcode] (vm);

        iterations++;
    }

    debug_print("Executed %u instructions\n", iterations);
}
