/**
 * @file bootloader.c
 * @author Ronaldo Carvalho (jronaldoac@hotmail.com)
 * @brief Desenvolvimento inicial de bootloader Hidro Solo (GateWay)
 * @version 0.1.0
 * @date 2025-07-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "bootloader.h"
#include <stdio.h>

#define BOOTLOADER_TIMEOUT_MS 5000

int main(void) 
{

    stdio_init_all();
    sleep_ms(6000);
    //Print será colocado em diretivas de compilação
    printf("\n=== BOOTLOADER RP2040 ===\n");
    printf("Versão: 0.1.0\n");
    // Inicialização básica
    if (!bootloader_init()) 
    {
        printf("[ERRO] Falha na inicialização\n");
        bootloader_recovery();
    }
    //MApeamento de memória
    printf("[INFO] Mapeamento de memória:\n");
    printf("[INFO]   Bootloader: 0x%08X - 0x%08X\n", FLASH_ADDR_BOOTLOADER, FLASH_ADDR_BOOTLOADER + BOOTLOADER_SIZE - 1);
    printf("[INFO]   Flags:      0x%08X - 0x%08X\n", FLASH_ADDR_FLAGS, FLASH_ADDR_FLAGS + FLAGS_SIZE - 1);
    printf("[INFO]   Factory FW: 0x%08X - 0x%08X\n", FLASH_ADDR_FACTORY_FW, FLASH_ADDR_FACTORY_FW + FIRMWARE_SIZE - 1);
    printf("[INFO]   Current FW: 0x%08X - 0x%08X\n", FLASH_ADDR_CURRENT_FW, FLASH_ADDR_CURRENT_FW + FIRMWARE_SIZE - 1);
    printf("[INFO]   Staging:    0x%08X - 0x%08X\n\n", FLASH_ADDR_STAGING_FW, FLASH_ADDR_STAGING_FW + FIRMWARE_SIZE - 1);
    printf("[INFO] Bootloader inicializado com sucesso\n");
    

    bootloader_jump_to(FLASH_ADDR_CURRENT_FW);
    printf("[INFO] Saltando para firmware atual (0x%08X)\n", FLASH_ADDR_CURRENT_FW);
    // Se o firmware atual não estiver presente, salta para o firmware de fábrica
    if (!bootloader_jump_to(FLASH_ADDR_CURRENT_FW))
    {
        printf("[ERRO] Firmware não encontrado ou inválido, tentando firmware de fábrica...\n");

    }
        
    printf("[INFO] Saltando para firmware factory (temporário)\n");
    bootloader_jump_to(FLASH_ADDR_FACTORY_FW);
    
    if(!bootloader_jump_to(FLASH_ADDR_FACTORY_FW))
    {
        // Se o firmware de fábrica também não estiver presente, entra em modo de recuperação
        printf("[ERRO] Firmware de fábrica não ausente ou inválido, entrando em modo de recuperação...\n");
        bootloader_recovery();
    }
    
    return 0;
}

bool bootloader_init(void) 
{
    printf("[INFO] Configurando watchdog...\n");
    watchdog_enable(BOOTLOADER_TIMEOUT_MS, 1);
    return true;
}

bool bootloader_jump_to(uint32_t fw_addr) 
{
    printf("[INFO] Preparando jump para 0x%08X\n", fw_addr);
    
    // Validação básica do endereço
    if (fw_addr < 0x10000000 || fw_addr >= 0x10200000) 
    {
        printf("[ERRO] Endereço inválido: 0x%08X\n", fw_addr);
        return false;
    }
    
    const uint32_t *vector_table = (const uint32_t*)fw_addr;
    uint32_t sp = vector_table[0];  // Stack pointer
    uint32_t pc = vector_table[1];  // Program counter
    
    printf("[INFO] Stack Pointer: 0x%08X\n", sp);
    printf("[INFO] Program Counter: 0x%08X\n", pc);
    
    // Validações ARM
    if (sp < 0x20000000 || sp > 0x20042000) 
    {
        printf("[ERRO] Stack pointer inválido\n");
        return false;
    }
    
    if ((pc & 1) == 0) 
    {
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

void bootloader_recovery(void) 
{
    printf("\n=== MODO RECOVERY ===\n");
    printf("Sistema em loop de recuperação\n");
    
    int counter = 0;
    while (1) 
    {
        printf("Recovery mode - contador: %d\n", counter++);
        sleep_ms(2000);
    }
}