#include "pico/stdlib.h"
#include <stdio.h>
#include "bootloader.h"

int main() {
    stdio_init_all();
    printf("Bootloader iniciado...\n");
    
    // L�gica b�sica para saltar para firmware atual ou factory
    // Pseudoc�digo de checagem e salto
    
    while (1) {
        tight_loop_contents();
    }
}
