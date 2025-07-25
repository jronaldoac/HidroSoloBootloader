#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>

// =================================================================================
// 1. ARQUITETURA DE MEMÓRIA FLASH (Conforme Especificado)
// =================================================================================
#define FLASH_XIP_BASE          0x10000000
#define ADDR_BOOTLOADER         0x10000000
#define SIZE_BOOTLOADER         (24 * 1024)
#define ADDR_FACTORY_FW         (ADDR_BOOTLOADER + SIZE_BOOTLOADER) // 0x10006000
#define SIZE_FACTORY_FW         (256 * 1024)
#define ADDR_METADATA           (ADDR_FACTORY_FW + SIZE_FACTORY_FW) // 0x10046000
#define SIZE_METADATA           (8 * 1024)
#define ADDR_CURRENT_FW         (ADDR_METADATA + SIZE_METADATA)     // 0x10048000
#define SIZE_CURRENT_FW         (256 * 1024)
#define ADDR_NEW_FW             (ADDR_CURRENT_FW + SIZE_CURRENT_FW) // 0x10088000
#define SIZE_NEW_FW             (256 * 1024)
#define ADDR_APP_DATA           (ADDR_NEW_FW + SIZE_NEW_FW)         // 0x100C8000
#define SIZE_APP_DATA           (256)
#define FW_MAX_PAYLOAD_SIZE     (256 * 1024)

// =================================================================================
// 2. CABEÇALHO DO FIRMWARE (Simplificado)
// =================================================================================
// Estrutura de 12 bytes alinhada para 32 bits.

typedef struct {
    uint32_t fw_size;       // Tamanho do payload do firmware em bytes.
    uint32_t fw_version;    // Versão do firmware (ex: 0x010200 para v1.2.0).
    uint16_t fw_crc16;      // CRC-16 do payload.
    uint16_t reserved;      // Padding para alinhamento de 32 bits.
} fw_header_t;


// =================================================================================
// 3. PROTÓTIPOS DAS FUNÇÕES
// =================================================================================
bool bootloader_validate_firmware(uint32_t fw_addr);
bool bootloader_update_firmware(uint32_t src_addr, uint32_t dst_addr);
void bootloader_clear_new_fw_area(void);
void bootloader_jump_to_current_fw(void);
void bootloader_enter_recovery(void);

#endif // BOOTLOADER_H
