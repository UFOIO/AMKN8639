#include "boot.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>
#if (BOOT_COM_RS232_ENABLE > 0)

#define RS232_CTO_RX_PACKET_TIMEOUT_MS (200u)
#define RS232_BYTE_TX_TIMEOUT_MS       (10u)

static blt_int8u xcpCtoReqPacket[BOOT_COM_RS232_RX_MAX_DATA + 3];
static blt_int8u xcpCtoRxLength;
static blt_bool  xcpCtoRxInProgress = BLT_FALSE;
static blt_int32u xcpCtoRxStartTime = 0;

static blt_bool Rs232ReceiveByte(blt_int8u *data);
static void     Rs232TransmitByte(blt_int8u data);

void Rs232Init(void)
{
    xcpCtoRxInProgress = BLT_FALSE;
    xcpCtoRxLength = 0;
    while (USART1->ISR & USART_ISR_RXNE_RXFNE) { (void)USART1->RDR; }
    USART1->ICR = 0xFFFFFFFFu;
}

static void Rs232TransmitByte(blt_int8u data)
{
    blt_int32u timeout = TimerGet() + RS232_BYTE_TX_TIMEOUT_MS;
    while (!(USART1->ISR & USART_ISR_TXE_TXFNF)) {
        if (TimerGet() > timeout) return;
    }
    USART1->TDR = data;
}

static blt_bool Rs232ReceiveByte(blt_int8u *data)
{
    if (USART1->ISR & USART_ISR_RXNE_RXFNE) {
        *data = (blt_int8u)(USART1->RDR & 0xFFu);
        return BLT_TRUE;
    }
    return BLT_FALSE;
}

void Rs232TransmitPacket(blt_int8u *data, blt_int8u len)
{
    blt_int8u i;
    for (i = 0; i < len; i++) Rs232TransmitByte(data[i]);
    /* Clear any ORE that occurred during TX */
    if (USART1->ISR & (1u << 3)) { USART1->ICR = (1u << 3); }
}

blt_bool Rs232ReceivePacket(blt_int8u *data, blt_int8u *len)
{
    blt_int8u rxByte;

    if (xcpCtoRxInProgress == BLT_FALSE)
    {
        /* Read available bytes until we find a valid length byte */
        while (USART1->ISR & USART_ISR_RXNE_RXFNE)
        {
            rxByte = (blt_int8u)(USART1->RDR & 0xFFu);
            if (rxByte > 0 && rxByte <= BOOT_COM_RS232_RX_MAX_DATA)
            {
                xcpCtoReqPacket[0] = rxByte;
                xcpCtoRxStartTime = TimerGet();
                xcpCtoRxLength = 0;
                xcpCtoRxInProgress = BLT_TRUE;
                break;
            }
        }
        /* Clear ORE if it occurred */
        if (USART1->ISR & (1u << 3)) { USART1->ICR = (1u << 3); }
    }
    
    if (xcpCtoRxInProgress == BLT_TRUE)
    {
        /* Clear ORE if it occurred between calls */
        if (USART1->ISR & (1u << 3)) { USART1->ICR = (1u << 3); }
        /* Read ALL available bytes in one go */
        while (USART1->ISR & USART_ISR_RXNE_RXFNE)
        {
            rxByte = (blt_int8u)(USART1->RDR & 0xFFu);
            xcpCtoReqPacket[xcpCtoRxLength + 1] = rxByte;
            xcpCtoRxLength++;
            if (xcpCtoRxLength == (blt_int8u)xcpCtoReqPacket[0])
            {
                CpuMemCopy((blt_int32u)data, (blt_int32u)&xcpCtoReqPacket[1], xcpCtoRxLength);
                xcpCtoRxInProgress = BLT_FALSE;
                *len = xcpCtoRxLength;
                return BLT_TRUE;
            }
        }
        /* Timeout check */
        if (TimerGet() > (xcpCtoRxStartTime + RS232_CTO_RX_PACKET_TIMEOUT_MS))
        {
            xcpCtoRxInProgress = BLT_FALSE;
        }
    }
    return BLT_FALSE;
}

#endif
