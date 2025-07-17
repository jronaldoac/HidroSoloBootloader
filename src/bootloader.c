#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "bootloader.h"
#include <stdio.h>

#define BOOTLOADER_TIMEOUT_MS 5000

int main(void) {
    stdio_init_all();
    sleep_ms(6000);
    
    printf("\n=== BOOTLOADER RP2040 ===\n");
    printf("Versão: 0.1.0\n");
    printf("Etapa 1: Estrutura base\n\n");
    
    // Inicialização básica
    if (!bootloader_init()) {
        printf("[ERRO] Falha na inicialização\n");
        bootloader_recovery();
    }
    
    printf("[INFO] Bootloader inicializado com sucesso\n");
    
    // Por enquanto, sempre salta para firmware factory
    printf("[INFO] Saltando para firmware factory (temporário)\n");
    bootloader_jump_to(FLASH_ADDR_FACTORY_FW);
    
    
    bootloader_recovery();
    return 0;
}

bool bootloader_init(void) {
    printf("[INFO] Configurando watchdog...\n");
    watchdog_enable(BOOTLOADER_TIMEOUT_MS, 1);
    
    printf("[INFO] Inicialização básica concluída\n");
    return true;
}

bool bootloader_jump_to(uint32_t fw_addr) {
    printf("[INFO] Preparando jump para 0x%08X\n", fw_addr);
    
    // Validação básica do endereço
    if (fw_addr < 0x10000000 || fw_addr >= 0x10200000) {
        printf("[ERRO] Endereço inválido: 0x%08X\n", fw_addr);
        return false;
    }
    
    const uint32_t *vector_table = (const uint32_t*)fw_addr;
    uint32_t sp = vector_table[0];  // Stack pointer
    uint32_t pc = vector_table[1];  // Program counter
    
    printf("[INFO] Stack Pointer: 0x%08X\n", sp);
    printf("[INFO] Program Counter: 0x%08X\n", pc);
    
    // Validações ARM
    if (sp < 0x20000000 || sp > 0x20042000) {
        printf("[ERRO] Stack pointer inválido\n");
        return false;
    }
    
    if ((pc & 1) == 0) {
        printf("[ERRO] Thumb bit não definido\n");
        return false;
    }
    
    printf("[INFO] Desabilitando watchdog e interrupções...\n");
    watchdog_disable();
    
    printf("[INFO] Executando jump...\n");
    sleep_ms(100);
    
    // Configurar MSP e jump
    __asm volatile("msr msp, %0" : : "r"(sp) : "memory");
    
    void (*app_reset_handler)(void) = (void (*)(void))(pc & ~1);
    app_reset_handler();
    
    return false; // Nunca retorna
}

void bootloader_recovery(void) {
    printf("\n=== MODO RECOVERY ===\n");
    printf("Sistema em loop de recuperação\n");
    
    int counter = 0;
    while (1) {
        printf("Recovery mode - contador: %d\n", counter++);
        sleep_ms(2000);
    }
}