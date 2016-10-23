#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>

#ifndef PATH_MAX
  #define PATH_MAX 1024
#endif

#include "std/owl_string.h"
#include "owl_list.h"
#include "alloc.h"
#include "term.h"

owl_term owl_file_pwd(vm_t *vm) {
  char *cwd = owl_alloc(vm, PATH_MAX);
  getcwd(cwd, PATH_MAX);
  return owl_string_from(cwd);
}

owl_term owl_file_ls(vm_t *vm, owl_term path) {
  DIR *d;
  struct dirent *dir;
  owl_term result = owl_list_init();
  char *dirname = owl_extract_ptr(path);

  d = opendir(dirname);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
        char *relpath = owl_alloc(vm, strlen(dir->d_name) + 1);
        strcpy(relpath, dir->d_name);
        owl_term entry = owl_string_from(relpath);
        result = owl_list_push(vm, result, entry);
      }
    }

    closedir(d);
  }

  return result;
}
