#include <stdio.h>
void some_function(int depth) {
    printf("Call number: %d\n", depth);

}


int main() {
    for(int i=0;;i++){
    some_function(i); 
    }
    return 0;
}
