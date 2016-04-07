#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    unsigned char *module_address = vm->code + vm->code_size;

    if (!code || !size || (vm->code_size + size > 0xFFFF))
        exit(1);

    memcpy(module_address, code, size);


    unsigned short name_size = (unsigned short) code[0];
    char *module_name = (char *) malloc(name_size + 1);

    strcpy(module_name, (char*) &code[1]);

    module_t *mod = malloc(sizeof(module_t));
    mod->name = module_name;
    mod->code_index = module_address;

    vm->ip = name_size + 2;
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

    debug_print("Executed %u instructions\n", iterations);
}
