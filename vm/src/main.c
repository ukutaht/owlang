#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "vm.h"

int run_file(const char *filename) {
    struct stat sb;

    if (stat(filename, &sb) != 0) {
        printf("Failed to read file: %s\n", filename);
        return 1;
    }

    int size = sb.st_size;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Failed to open program-file %s\n", filename);
        return 1;
    }

    unsigned char *code = malloc(size);
    memset(code, '\0', size);

    if (!code) {
        printf("Failed to allocate memory for program-file %s\n", filename);
        fclose(fp);
        return 1;
    }

    fread(code, 1, size, fp);
    fclose(fp);

    vm_t *vm = vm_new();

    if (!vm) {
        printf("Failed to create virtual machine instance.\n");
        return 1;
    }

    vm_load_module(vm, code, size);
    vm_run(vm);


    free(code);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s input-file\n", argv[0]);
        return 0;
    }

    return run_file(argv[1]);
}
