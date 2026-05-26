#include <stdio.h>
#include <stdlib.h>

#include "math_functions.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Please enter 2 numbers.\n");
    return 1;
  }

  int x = atoi(argv[1]);
  int y = atoi(argv[2]);

  printf("Addition: %d\n", add(x, y));
  printf("Subtraction: %d\n", subtract(x, y));

  return 0;
}