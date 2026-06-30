#ifndef BLT_CONF_H
#define BLT_CONF_H

/* CPU - HSI 64MHz */
#define BOOT_CPU_XTAL_SPEED_KHZ          (64000)
#define BOOT_CPU_SYSTEM_SPEED_KHZ        (64000)
#define BOOT_CPU_BYTE_ORDER_MOTOROLA     (0)
#define BOOT_CPU_USER_PROGRAM_START_HOOK (1)

/* RS232 - UART1 (PA9/PA10) */
#define BOOT_COM_RS232_ENABLE             (1)
#define BOOT_COM_RS232_CHANNEL_INDEX      (0)
#define BOOT_COM_RS232_BAUDRATE           (115200)
#define BOOT_COM_RS232_TX_MAX_DATA        (128)
#define BOOT_COM_RS232_RX_MAX_DATA        (128)

#define BOOT_COM_NET_ENABLE               (0)
#define BOOT_COM_CAN_ENABLE               (0)
#define BOOT_COM_USB_ENABLE               (0)
#define BOOT_COM_DEFERRED_INIT_ENABLE     (0)

#define BOOT_XCP_RS232_ENABLE             (BOOT_COM_RS232_ENABLE)

/* Backdoor - 100ms */
#define BOOT_BACKDOOR_ENTRY_TIMEOUT_MS    (5000)
#define BOOT_BACKDOOR_HOOKS_ENABLE        (0)

/* NVM - 2MB flash */
#define BOOT_NVM_SIZE_KB                  (2048)
#define BOOT_NVM_HOOKS_ENABLE             (0)
#define BOOT_NVM_CHECKSUM_HOOKS_ENABLE    (1)
#define BOOT_NVM_USER_PROG_BASEADDR       (0x08020000)

#define BOOT_FILE_SYS_ENABLE              (0)
#define BOOT_FLASH_VECTOR_TABLE_CS_OFFSET (0x298)
#define BOOT_FLASH_CUSTOM_LAYOUT_ENABLE   (0)
#define BOOT_EVENTS_ENABLE                (0)
#define BOOT_INFO_TABLE_ENABLE            (0)
#define BOOT_COP_HOOKS_ENABLE             (0)

#endif


