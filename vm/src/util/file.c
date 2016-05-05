#include <string.h>
#include <stdlib.h>

#include "file.h"

// Get module name from filename. Examples:
// _build/arithmetic_test.owlc -> arithmetic_test
// arithmetic_test.owlc -> arithmetic_test
char* module_name_from_filename(char *filename) {
  char *module_name = rindex(filename, '/');

  if (module_name == NULL) {
    module_name = filename;
  } else {
    module_name++;
  }

  char *cpy = malloc(strlen(module_name));
  strcpy(cpy, module_name);
  *rindex(cpy, '.') = '\0';

  return cpy;
}
