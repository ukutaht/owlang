#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "opcodes.h"

vm_t *vm_new(unsigned char *code, unsigned int size) {
    vm_t *vm;
    int i;

    if (!code || !size || (size > 0xFFFF))
        return NULL;

    vm = malloc(sizeof(struct vm));
    if (!vm)
        return NULL;
    memset(vm, '\0', sizeof(struct vm));

    /**
     * Allocate 64k for the program.
     */
    vm->code = malloc(0xFFFF);
    if (vm->code == NULL) {
        free(vm);
        return NULL;
    }

    vm->ip = 0;
    vm->ret_address = 0;
    vm->running = true;
    vm->size = size;


    /**
     * Zero the RAM we've allocated, and then copy the user's program to
     * the start of it.
     *
     * This means there is a full 64k address-space and the user can
     * have fun writing self-modifying code, & etc.
     *
     */
    memset(vm->code, '\0', 0xFFFF);
    memcpy(vm->code, code, size);


    /**
     * Explicitly zero each register and set to be a number.
     */
    for (i = 0; i < REGISTER_COUNT; i++) {
        vm->registers[i].type = INTEGER;
        vm->registers[i].content.integer = 0;
        vm->registers[i].content.string = NULL;
    }


    /**
     * Setup our default opcode-handlers
     */
    opcode_init(vm);

    return vm;
}

/**
 *  Main virtual machine execution loop
 *
 *  This function will walk through the code passed to the constructor
 * and attempt to execute each bytecode instruction.
 *
 */
void vm_run(vm_t * cpup) {
    int iterations = 0;

    /**
     * If we're called without a valid CPU then we should abort.
     */
    if (!cpup)
        return;


    /**
     * The code will start executing from offset 0.
     */
    cpup->ip = 0;


    /**
     * Run continuously.
     *
     * In practice this means until an EXIT instruction is encountered,
     * which will set the "running"-flag to be false.
     *
     * However the system can cope with IP wrap-around.
     */
    while (cpup->running == true) {
        /**
         * Wrap IP on the 64k boundary, if required.
         */
        if (cpup->ip >= 0xFFFF)
            cpup->ip = 0;


        /**
         * Lookup the instruction at the instruction-pointer.
         */
        int opcode = cpup->code[cpup->ip];
        /**
         * Call the opcode implementation, if defined.
         */
        if (cpup->opcodes[opcode] != NULL)
            cpup->opcodes[opcode] (cpup);

        /**
         * NOTE: At this point you might be looking for
         *       a line of the form : cpup->ip += 1;
         *
         *       However this is NOT REQUIRED as each opcode
         *       will have already updated the (global) instruction
         *       pointer.
         *
         *       This is neater because each opcode knows how long it is,
         *       and will probably have bumped the IP to read the register
         *       number, or other arguments.
         *
         */
        iterations++;
    }

    DEBUG("Executed %u instructions\n", iterations);
}
