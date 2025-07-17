#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>

#define FLASH_ADDR_CURRENT_FW   0x10009000 // Endere�o do firmware atual corresponde a 0x10000000 + 0x9000 em hexa e em decimal corresponde  
#define FLASH_ADDR_FACTORY_FW   0x10000000 // 
#define FLAGS_MAGIC_NUMBER      0xDEADBEEF
#define HASH_SIZE               32

typedef enum {
    FIRMWARE_STATUS_IDLE = 0, // Nenhum firmware carregado
    FIRMWARE_STATUS_RECEIVED, // Firmware recebido, aguardando valida��o
    FIRMWARE_STATUS_VALIDATED, // firmware validado e pronto para ser executado
    FIRMWARE_STATUS_ERROR // Erro na valida��o do firmware
} FirmwareStatus_t;

typedef struct {
    uint32_t magic; // N�mero m�gico para validar a estrutura
    FirmwareStatus_t status; // Status do firmware
    uint16_t crc16; // CRC16 do firmware
    uint8_t sha256[HASH_SIZE]; // Hash SHA-256 do firmware
    uint32_t firmware_size; // Tamanho do firmware
    uint32_t fw_version; // Vers�o do firmware
} BootloaderFlags_t;

// Prototipos de fun��es do bootloader
bool bootloader_init(void); // Inicializa o bootloader
bool bootloader_read_flags(BootloaderFlags_t *flags); // L� os flags do bootloader
bool bootloader_validate_firmware(const uint8_t* fw_addr, uint32_t fw_size, const BootloaderFlags_t *flags); 
bool bootloader_jump_to(uint32_t fw_addr);
void bootloader_recovery(void);

#endif // BOOTLOADER_H
