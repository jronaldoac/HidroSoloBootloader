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

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
	#define DBG_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DBG_PRINTF(...) do {} while(0)
#endif
#define BOOTLOADER_TIMEOUT_MS 5000

int main(void) 
{

    stdio_init_all();
    sleep_ms(6000);
    //Print será colocado em diretivas de compilação
    DBG_PRINTF("\n=== BOOTLOADER RP2040 ===\n");
    DBG_PRINTF("Versão: 0.1.0\n");
    // Inicialização básica
    if (!bootloader_init()) 
    {
        DBG_PRINTF("[ERRO] Falha na inicialização\n");
        bootloader_recovery();
    }
    //MApeamento de memória
    DBG_PRINTF("[INFO] Mapeamento de memória:\n");
    DBG_PRINTF("[INFO]   Bootloader: 0x%08X - 0x%08X\n", FLASH_ADDR_BOOTLOADER, FLASH_ADDR_BOOTLOADER + BOOTLOADER_SIZE - 1);
    DBG_PRINTF("[INFO]   Flags:      0x%08X - 0x%08X\n", FLASH_ADDR_FLAGS, FLASH_ADDR_FLAGS + FLAGS_SIZE - 1);
    DBG_PRINTF("[INFO]   Factory FW: 0x%08X - 0x%08X\n", FLASH_ADDR_FACTORY_FW, FLASH_ADDR_FACTORY_FW + FIRMWARE_SIZE - 1);
    DBG_PRINTF("[INFO]   Current FW: 0x%08X - 0x%08X\n", FLASH_ADDR_CURRENT_FW, FLASH_ADDR_CURRENT_FW + FIRMWARE_SIZE - 1);
    DBG_PRINTF("[INFO]   Staging:    0x%08X - 0x%08X\n\n", FLASH_ADDR_STAGING_FW, FLASH_ADDR_STAGING_FW + FIRMWARE_SIZE - 1);
    
    DBG_PRINTF("[INFO] Bootloader inicializado com sucesso\n");
    
    // Prioridade 1: Tentar firmware atual (principal)
    DBG_PRINTF("[INFO] Saltando para firmware atual (0x%08X)\n", FLASH_ADDR_CURRENT_FW);
    bootloader_jump_to(FLASH_ADDR_CURRENT_FW);
    
    // Se chegou aqui, o firmware atual falhou
    DBG_PRINTF("[ERRO] Firmware atual falhou ou não encontrado\n");
    
    // Prioridade 2: Tentar firmware de fábrica (fallback)
    DBG_PRINTF("[INFO] Tentando firmware de fábrica (0x%08X)\n", FLASH_ADDR_FACTORY_FW);
    bootloader_jump_to(FLASH_ADDR_FACTORY_FW);
    
    // Se chegou aqui, ambos falharam
    DBG_PRINTF("[ERRO] Ambos firmwares falharam\n");
    DBG_PRINTF("[INFO] Entrando em modo de recuperação...\n");
    bootloader_recovery();
    
    return 0;
}

bool bootloader_init(void) 
{
    DBG_PRINTF("[INFO] Configurando watchdog...\n");
    watchdog_enable(BOOTLOADER_TIMEOUT_MS, 1);
    return true;
}

bool bootloader_jump_to(uint32_t fw_addr) 
{
    DBG_PRINTF("[INFO] Preparando jump para 0x%08X\n", fw_addr);
    
    // Validação básica do endereço
    if (fw_addr < 0x10000000 || fw_addr >= 0x10200000) 
    {
        DBG_PRINTF("[ERRO] Endereço inválido: 0x%08X\n", fw_addr);
        return false;
    }
    
    const uint32_t *vector_table = (const uint32_t*)fw_addr;
    uint32_t sp = vector_table[0];  // Stack pointer
    uint32_t pc = vector_table[1];  // Program counter
    
    DBG_PRINTF("[INFO] Stack Pointer: 0x%08X\n", sp);
    DBG_PRINTF("[INFO] Program Counter: 0x%08X\n", pc);
    
    // Validações ARM
    if (sp < 0x20000000 || sp > 0x20042000) 
    {
        DBG_PRINTF("[ERRO] Stack pointer inválido\n");
        return false;
    }
    
    if ((pc & 1) == 0) 
    {
        DBG_PRINTF("[ERRO] Thumb bit não definido\n");
        return false;
    }
    
    DBG_PRINTF("[INFO] Desabilitando watchdog e interrupções...\n");
    watchdog_disable();
    
    DBG_PRINTF("[INFO] Executando jump...\n");
    sleep_ms(100);
    
    // Configurar MSP e jump
    __asm volatile("msr msp, %0" : : "r"(sp) : "memory");
    
    void (*app_reset_handler)(void) = (void (*)(void))(pc & ~1);
    app_reset_handler();
    
    return false; // Nunca retorna
}

void bootloader_recovery(void) 
{
    DBG_PRINTF("\n=== MODO RECOVERY ===\n");
    DBG_PRINTF("Sistema em loop de recuperação\n");
    
    int counter = 0;
    while (1) 
    {
        DBG_PRINTF("Recovery mode - contador: %d\n", counter++);
        sleep_ms(2000);
    }
}