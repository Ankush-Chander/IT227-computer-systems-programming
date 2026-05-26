#include <stdio.h>
void causeSegmentationFault(int depth) {
    // function calls itself by appending the argument
    // note there is no stopping condition in the recursion, hence it will keep on calling itself forever.
    // thereby expanding stack
    printf("Recursion depth: %d\n", depth);
    causeSegmentationFault(depth + 1); // Recursive call with increased depth
}
int main() {
    causeSegmentationFault(1); // Start the recursion
    return 0;
}
