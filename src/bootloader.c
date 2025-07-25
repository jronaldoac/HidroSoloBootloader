/**
 * @file bootloader.c
 * @brief Bootloader para RP2040 com validação por CRC16.
 * @version 2.2.1 (Corrigido)
 * @date 2025-07-25
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "pico/bootrom.h"
#include "hardware/sync.h"  
#include "bootloader.h"

#define DBG_PRINTF(...) printf(__VA_ARGS__)

// Protótipo da nova função de CRC-16
static uint16_t calculate_crc16_ccitt(const uint8_t *data, uint32_t length);

int main(void) {
    stdio_init_all();
    sleep_ms(2000);
    DBG_PRINTF("\n=== BOOTLOADER v2.2.1 (CRC16) INICIADO ===\n");
   // watchdog_enable(8000, 1);

    //Verifica se o firmware atual é válido
    DBG_PRINTF("[FLUXO] Verificando Novo FW em 0x%08X...\n", ADDR_NEW_FW);
    if (bootloader_validate_firmware(ADDR_NEW_FW)) {
        DBG_PRINTF("[FLUXO] FW em Staging VALIDO. Tentando atualizar...\n");

        if (bootloader_update_firmware(ADDR_NEW_FW, ADDR_CURRENT_FW)) {
            DBG_PRINTF("[FLUXO] Atualizacao bem-sucedida.\n");
            bootloader_clear_new_fw_area();
        } else {
            DBG_PRINTF("[ERRO] Falha ao gravar o novo firmware!\n");
            bootloader_clear_new_fw_area();
        }
    } else {
        DBG_PRINTF("[FLUXO] Area de Staging vazia ou com FW invalido.\n");
    }

    // Tentar iniciar o firmware principal
    DBG_PRINTF("[FLUXO] Tentando iniciar FW Atual em 0x%08X...\n", ADDR_CURRENT_FW);
    if (bootloader_validate_firmware(ADDR_CURRENT_FW)) {
        bootloader_jump_to_current_fw();
    }
    
    // FALLBACK 1: Tentar restaurar para fw de Fábrica
    DBG_PRINTF("[ERRO] FW Atual invalido. Tentando restaurar do FW de Fabrica (0x%08X)...\n", ADDR_FACTORY_FW);
    if (bootloader_validate_firmware(ADDR_FACTORY_FW)) {
        if (bootloader_update_firmware(ADDR_FACTORY_FW, ADDR_CURRENT_FW)) {
            DBG_PRINTF("[INFO] Restauracao concluida. Reiniciando para aplicar...\n");
            watchdog_reboot(0, 0, 100);
        }
    }

    // FALLBACK 2: Modo de recuperação final
    DBG_PRINTF("[ERRO FATAL] Nenhuma imagem de FW valida. Entrando em modo de recuperacao.\n");
    bootloader_enter_recovery();
    return 0;
}

bool bootloader_validate_firmware(uint32_t fw_addr) {
    const fw_header_t *header = (const fw_header_t *)fw_addr;

    // 1. Validação de sanidade do tamanho do firmware
    if (header->fw_size == 0 || header->fw_size > FW_MAX_PAYLOAD_SIZE || header->fw_size == 0xFFFFFFFF) {
        return false;
    }

    // 2. Validação do CRC-16
    const uint8_t *fw_payload = (const uint8_t *)(fw_addr + sizeof(fw_header_t));
    uint16_t calculated_crc = calculate_crc16_ccitt(fw_payload, header->fw_size);

    if (calculated_crc != header->fw_crc16) {
        DBG_PRINTF("[VALIDATE] CRC16 falhou em 0x%08X. Esperado: 0x%04X, Calculado: 0x%04X\n", fw_addr, header->fw_crc16, calculated_crc);
        return false;
    }
    
    return true;
}

bool bootloader_update_firmware(uint32_t src_addr, uint32_t dst_addr) {
    const fw_header_t *header = (const fw_header_t *)src_addr;
    uint32_t total_size = sizeof(fw_header_t) + header->fw_size;
    uint32_t erase_size = (total_size + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
    
    uint32_t dst_offset = dst_addr - FLASH_XIP_BASE;

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(dst_offset, erase_size);
    flash_range_program(dst_offset, (const uint8_t*)src_addr, total_size);
    restore_interrupts(ints);
    
    return bootloader_validate_firmware(dst_addr);
}

void bootloader_clear_new_fw_area(void) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(ADDR_NEW_FW - FLASH_XIP_BASE, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
}

__attribute__((noreturn))
void bootloader_jump_to_current_fw(void) {
    // A validação do firmware já foi feita, agora apenas validamos o salto
    const uint32_t *vector_table = (const uint32_t *)(ADDR_CURRENT_FW + sizeof(fw_header_t));
    uint32_t stack_ptr = vector_table[0];
    uint32_t entry_point = vector_table[1];

    // Validação final do vector table antes do salto
    if (stack_ptr < 0x20000000 || stack_ptr > 0x20042000 || (entry_point & 1) == 0) {
        DBG_PRINTF("[ERRO] Vector table invalido no FW Atual. Abortando salto.\n");
        // Força um reboot para tentar o recovery de fábrica
        watchdog_reboot(0, 0, 100);
    }
    
    DBG_PRINTF("[JUMP] Saltando para FW Atual. SP: 0x%08X, PC: 0x%08X\n", stack_ptr, entry_point);
    watchdog_disable();
    save_and_disable_interrupts();
    __asm volatile ("msr msp, %[sp]\n" "bx %[pc]\n" : : [sp] "r" (stack_ptr), [pc] "r" (entry_point));
    while(1);
}

__attribute__((noreturn))
void bootloader_enter_recovery(void) {
    //reset_usb_boot(0, 0);
    while(1);
}

static uint16_t calculate_crc16_ccitt(const uint8_t *data, uint32_t length) {
    uint16_t crc = 0xFFFF; // Valor inicial para CCITT-FALSE
    while (length--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
