#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>

#ifndef PATH_MAX
  #define PATH_MAX 1024
#endif

#include "term.h"
#include "std/owl_string.h"
#include "owl_list.h"
#include "alloc.h"

owl_term owl_file_pwd(void) {
  char *cwd = owl_alloc(PATH_MAX);
  getcwd(cwd, PATH_MAX);
  return owl_string_from(cwd);
}

owl_term owl_file_ls(owl_term path) {
  DIR *d;
  struct dirent *dir;
  owl_term result = owl_list_init();
  char *dirname = owl_extract_ptr(path);

  d = opendir(dirname);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
        owl_term entry = owl_string_from(dir->d_name);
        result = owl_list_push(result, entry);
      }
    }

    closedir(d);
  }

  return result;
}
