#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>

// === MAPEAMENTO DE MEMÓRIA FLASH ===
#define FLASH_BASE              0x10000000

// Endereços das diferentes áreas
#define FLASH_ADDR_BOOTLOADER   0x10000000    // Bootloader (32KB)
#define FLASH_ADDR_FLAGS        0x10008000    // Flags/Metadados (4KB)
#define FLASH_ADDR_FACTORY_FW   0x10009000    // Firmware de Fábrica (512KB) - FALLBACK
#define FLASH_ADDR_CURRENT_FW   0x10089000    // Firmware Atual (512KB) - PRINCIPAL
#define FLASH_ADDR_STAGING_FW   0x10109000    // Staging (512KB) - TEMP

// Tamanhos das áreas
#define BOOTLOADER_SIZE         0x8000        // 32KB
#define FLAGS_SIZE              0x1000        // 4KB
#define FIRMWARE_SIZE           0x80000       // 512KB cada área

#define HASH_SIZE              32             // SHA-256 hash size in bytes

typedef enum {
    FIRMWARE_STATUS_IDLE = 0, // Nenhum firmware carregado
    FIRMWARE_STATUS_RECEIVED, // Firmware recebido, aguardando validação
    FIRMWARE_STATUS_VALIDATED, // firmware validado e pronto para ser executado
    FIRMWARE_STATUS_ERROR // Erro na validação do firmware
} FirmwareStatus_t;

typedef struct {
    FirmwareStatus_t status; // Status do firmware
    uint16_t crc16; // CRC16 do firmware
    uint8_t sha256[HASH_SIZE]; // Hash SHA-256 do firmware
    uint32_t firmware_size; // Tamanho do firmware
    uint32_t fw_version; // Versão do firmware
} BootloaderFlags_t;

// Prototipos de funções do bootloader
bool bootloader_init(void); // Inicializa o bootloader
bool bootloader_read_flags(BootloaderFlags_t *flags); // Lê os flags do bootloader
bool bootloader_validate_firmware(const uint8_t* fw_addr, uint32_t fw_size, const BootloaderFlags_t *flags); 
bool bootloader_jump_to(uint32_t fw_addr);
void bootloader_recovery(void);

#endif // BOOTLOADER_H
