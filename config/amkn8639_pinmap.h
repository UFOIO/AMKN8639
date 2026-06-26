/************************************************************************************
 * File       : amkn8639_pinmap.h   (canonical, in targets/AMKN8639/config/)
 * Project    : AMKN8639 Industrial Control Board / 2026 InnerMongolia Light Hangar
 * MCU        : STM32H743XIH6 @480MHz (core module)
 * Encoding   : GB2312 / ANSI (NOT UTF-8)
 *
 * Description:
 *   AMKN8639 board-level IO -> MCU pin / channel bridge for business layer.
 *
 * Notes:
 *   1. This file ONLY bridges semantic names -> IOConfig.h macros -> MCU pins.
 *      Real pin definitions live in AMKN8639_IOConfig.h.
 *   2. Business code (user_app.c / user_ioapp.c / user_adcapp.c / user_pwmapp.c /
 *      TaskStepMotor.c etc.) includes this header to use uniform semantic names.
 *   3. Anything tagged [TODO] is awaiting user confirmation in the hardware table.
 *
 * Modifiy History:
 *   1. Version: 1.10  2026-06-25
 *      - BMS read via UART; Hall on AI; Y axis A/B same axis; PWM4 reserved
 *   2. Version: 1.20  2026-06-26
 *      - All Chinese -> ASCII (GB2312 source compatibility)
 *      - UART1 -> RK3568 / USB1; UART3 -> JP20 BMS; UART_MODBUS shares UART1
 *      - Add AI1 (Hall voltage) and AI2 (Hall current)
 *      - Document DO high-active polarity and YModem firmware upgrade path
 *************************************************************************************/
#ifndef __AMKN8639_PINMAP_H
#define __AMKN8639_PINMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AMKN8639_IOConfig.h"
/* Note: this file does NOT depend on AMKN8639_Config.h. */

/*====================================================================================
 * 1. Emergency-stop (safety chain)
 *   DI1 = E-stop input; DO9 (K1 relay) = E-stop output; ALARM = buzzer; RUN_LED = run
 *====================================================================================*/
#define EMG_DI               DI1     /* E-stop input   JP6.2   PD11 */
#define EMG_DI_PORT          DI1
#define EMG_DI_PIN           DI1
#define EMG_DI_EXTI_LINE     11      /* EXTI11 candidate */

#define EMG_OUT_RELAY        DO9     /* E-stop relay K1   JP7.2   PB7 (COMA) */
#define ALARM_BEEP           ALARM   /* Buzzer                 PG3 */
#define RUN_LED              LED1    /* Run indicator          PI8 */

/*====================================================================================
 * 2. BMS / Hall sensing
 *   - BMS: voltage / current / status read via UART protocol (not AI).
 *   - Hall sensor (2 lines):
 *       * Voltage: AI1 (PC2, JP1.2)  0~10V input
 *       * Current: AI2 (PC3, JP1.3)  0~20mA input (250R + divider on board)
 *====================================================================================*/
#define AI_HALL_VOLT         AI1     /* Hall voltage          PC2 (JP1.2) */
#define AI_HALL_CURR         AI2     /* Hall current          PC3 (JP1.3) */

/* BMS UART path is defined in section 4. */

/*====================================================================================
 * 3. Stepper motors (PWM + DIR + ENA)
 *   PWM1 (TIM1)  -> X axis door-open motor (JP8)
 *   PWM2 (TIM2)  -> Y axis motor A         (JP12)
 *   PWM3 (TIM3)  -> Y axis motor B         (JP13)  -- A + B on the same axis
 *   PWM4 (TIM17) -> Reserved               (JP4)
 *
 *   External motor driver needs FWD / REV direction lines, controlled by
 *   DO1..DO4 (high-active: 1 = FWD/REV enabled, 0 = disabled).
 *====================================================================================*/
#define MOTOR_X_PUL          PWM1_PUL    /* X axis pulse  PA8 */
#define MOTOR_X_DIR          PWM1_DIR    /* X axis dir   PJ13 */
#define MOTOR_X_ENA          PWM1_ENA    /* X axis ena   PJ14 */
#define MOTOR_X_TIM          TIM1_ID
#define MOTOR_X_CH           1

#define MOTOR_YA_PUL         PWM2_PUL    /* Y axis A pulse  PA15 */
#define MOTOR_YA_DIR         PWM2_DIR    /* Y axis A dir    PD3 */
#define MOTOR_YA_ENA         PWM2_ENA    /* Y axis A ena    PD4 */
#define MOTOR_YA_TIM         TIM2_ID
#define MOTOR_YA_CH          1

#define MOTOR_YB_PUL         PWM3_PUL    /* Y axis B pulse  PC6 */
#define MOTOR_YB_DIR         PWM3_DIR    /* Y axis B dir    PC7 */
#define MOTOR_YB_ENA         PWM3_ENA    /* Y axis B ena   PH15 */
#define MOTOR_YB_TIM         TIM3_ID
#define MOTOR_YB_CH          1

#define MOTOR_RESV_PUL       PWM4_PUL    /* Reserved motor pulse  PF7 */
#define MOTOR_RESV_DIR       PWM4_DIR    /* PJ0 */
#define MOTOR_RESV_ENA       PWM4_ENA    /* PJ1 */
#define MOTOR_RESV_TIM       TIM17_ID
#define MOTOR_RESV_CH        1

/* Encoder (1 group FCLK1, PI5/PI6/PI7, TIM8) -- not assigned yet */
#define ENC_FCLK_CH1         FCLK1_CH1   /* PI5 (F1A+/-) */
#define ENC_FCLK_CH2         FCLK1_CH2   /* PI6 (F1B+/-) */
#define ENC_FCLK_CH3         FCLK1_CH3   /* PI7 (F1Z+/-) */
#define ENC_TIM              TIM8_ID

/*====================================================================================
 * 4. UART / RS485 / RS232 / CAN
 *====================================================================================*/
/* UART1 = PA9/PA10 -> USB1 (USB-UART) -> RK3568 (debug + business protocol) */
#define UART_RK3568          1
#define UART_RK3568_TX       UART1_TX   /* PA9  */
#define UART_RK3568_RX       UART1_RX   /* PA10 */

#define UART_DEBUG           1          /* Debug console reuses UART1 */
#define UART_NMEA            1          /* NMEA uplink (if GPS attached) */

/* Modbus RTU default on UART1 (shares UART1 with RK3568) */
#define UART_MODBUS          1
#define UART_MODBUS_TX       UART1_TX
#define UART_MODBUS_RX       UART1_RX

/* UART3 = PB10/PB11 -> JP20 RS232 -> aircraft BMS (via RS232-to-TTL converter) */
#define UART_BMS             3
#define UART_BMS_TX          UART3_TX   /* PB10 */
#define UART_BMS_RX          UART3_RX   /* PB11 */

/* UART4 = PA12/PA11 -> JP9 RS232 -> LCD */
#define UART_LCD             4
#define UART_LCD_TX          UART4_TX   /* PA12 */
#define UART_LCD_RX          UART4_RX   /* PA11 */

/* RS485 channels (reserved for TH sensor etc.) */
#define RS485_CH1_TX         UART6_TX   /* PG14 -> A1+/B1- */
#define RS485_CH1_RX         UART6_RX   /* PG9  */
#define RS485_CH1_DIR        UART6_DIR  /* PD7  */

#define RS485_CH2_TX         UART7_TX   /* PB4 -> A2+/B2- */
#define RS485_CH2_RX         UART7_RX   /* PB3  */
#define RS485_CH2_DIR        UART7_DIR  /* PG10 */

#define RS485_CH3_TX         UART8_TX   /* PJ8 -> A3+/B3- */
#define RS485_CH3_RX         UART8_RX   /* PJ9  */
#define RS485_CH3_DIR        UART8_DIR  /* PJ7  */

/* BMS communication port -- JP20 RS232 -> RS232-to-TTL -> aircraft TTL */
#define BMS_UART_ID          UART3_ID
#define BMS_UART_TX          UART3_TX   /* PB10 */
#define BMS_UART_RX          UART3_RX   /* PB11 */

/* CAN1 / CAN2 */
#define CAN1_TX_PORT         CAN1_TX    /* PH13 */
#define CAN1_RX_PORT         CAN1_RX    /* PH14 */
#define CAN2_TX_PORT         CAN2_TX    /* PB6  */
#define CAN2_RX_PORT         CAN2_RX    /* PB5  */

/*====================================================================================
 * 5. Relays (DO9~DO16, K1~K8) on JP7 (COMA) + JP18 (COMB)
 *   DO1~DO8 are fast DO (not relays).
 *====================================================================================*/
#define RELAY_K1_EMG         DO9        /* E-stop output */
#define RELAY_K1             DO9        /* PB7  COMA */
#define RELAY_K2             DO10       /* PI4  COMA */
#define RELAY_K3             DO11       /* PE2  COMA */
#define RELAY_K4             DO12       /* PE6  COMA */
#define RELAY_K5             DO13       /* PE4  COMB */
#define RELAY_K6             DO14       /* PE5  COMB */
#define RELAY_K7             DO15       /* PE3  COMB */
#define RELAY_K8             DO16       /* PI9  COMB */

/*====================================================================================
 * 6. Storage / bus
 *====================================================================================*/
#define FLASH_W25Q_CS        QSPI_BK1_CS    /* PG6 */
#define FLASH_W25Q_CLK       QSPI_BK1_CLK   /* PF10 */
#define FLASH_W25Q_IO0       QSPI_BK1_IO0   /* PF8 (MOSI) */
#define FLASH_W25Q_IO1       QSPI_BK1_IO1   /* PF9 (MISO) */

#define EEPROM_I2C           I2C2_ID
#define EEPROM_SCL           I2C2_SCL       /* PH4 */
#define EEPROM_SDA           I2C2_SDA       /* PH5 */

#define SD_BUS               SDMMC1_ID
#define SD_CMD               SDMMC_CMD      /* PD2 */
#define SD_CLK               SDMMC_CLK      /* PC12 */
#define SD_D0                SDMMC_D0       /* PC8  */
#define SD_D1                SDMMC_D1       /* PC9  */
#define SD_D2                SDMMC_D2       /* PC10 */
#define SD_D3                SDMMC_D3       /* PC11 */

/*====================================================================================
 * 7. Network / USB / LCD
 *====================================================================================*/
#define ETH_PHY_RST          ETH_RESET      /* PI10 */
#define USB_FS_DP            USB_DP         /* PB15 */
#define USB_FS_DM            USB_DM         /* PB14 */
#define USB_FS_VBUS          USB_VBUS       /* PB13 */
#define USB_FS_PWR           USB_PWR        /* PB12 */
#define LCD_BL_PIN           LCD_BL         /* PF6 */

/*====================================================================================
 * 8. EXTI hint (NVIC / HAL config for E-stop external interrupt)
 *   Default Config.h EXTI0~15 EN=0 (disabled). To enable E-stop interrupt:
 *     EXTI11_EN = 1;  EXTI11_IO = PD11;  EXTI11_MODE = 1;  // falling edge
 *====================================================================================*/
#define EMG_EXTI_IO_CANDIDATE PD11            /* PD11 / PD12 / PD13 / PH8..PH12 candidates */
#define EMG_EXTI_IRQN        EXTI15_10_IRQn   /* H743 EXTI10~15 share one IRQ */

/*====================================================================================
 * 9. Firmware upgrade path (YModem via UART)
 *   To enable OTA over UART1 (RK3568 path):
 *     IAP_EN = 1;  IAP_TFTP_EN = 0;  IAP_YMODEM_EN = 1;
 *   In Config.h, set IAP_UART_ID = UART1_ID;  IAP_BAUD = 115200;
 *   On host side: RK3568 runs `sx -vv -b amkn8639.bin < /dev/ttyUSB0` (YModem-sx)
 *   [TODO] Confirm whether libapp's IAP module supports YModem or only TFTP.
 *====================================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* __AMKN8639_PINMAP_H */
/************************************************************************************
 * End of file
 *************************************************************************************/