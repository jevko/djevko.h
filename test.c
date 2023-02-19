#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "djevko.h"

int main() {
  printf("%s\n", escape("abc"));
  printf("%s\n", escape("[]"));
  printf("%s\n", escape("[]`"));
  printf("%s\n", escape("`abc`"));
  printf("%s\n", escape("=`abc`="));

  return 0;
}
