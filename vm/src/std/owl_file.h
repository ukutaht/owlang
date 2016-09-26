#ifndef OWL_FILE_H
#define OWL_FILE_H

#include "owl.h"

owl_term owl_file_pwd(vm_t *vm);
owl_term owl_file_ls(vm_t *vm, owl_term path);

#endif  // OWL_FILE_H
