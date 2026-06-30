;******************************************************************************
;  File name: startup_boot.s
;  Project  : AMKN8639 self-developed Bootloader
;  Description: Minimal startup for Bootloader
;                - Stack top + Reset_Handler + Spurious for all other entries
;                - Linked at 0x08000000, Reset_Handler jumps to Bootloader_Main
;                - 体积必须 < 64KB
;
;  Modify History:
;   1. Version: 1.0
;      Date:    2026.6.22
;      Modify:  Phase 3.2 - create file (with MACRO for vector fill)
;******************************************************************************

                PRESERVE8
                AREA   BOOT_VECT, CODE, READONLY
                THUMB
                ENTRY

;******************************************************************************
;  Imports / Exports
;******************************************************************************
                IMPORT  Bootloader_Main
                IMPORT  SpuriousHandler_ISR
; Modify 2026.6.24 v23: BL 加 USART3 RXNE 中断, 注册向量 IRQ 39 (H743 中 USART3_IRQn=39)
                IMPORT  USART3_IRQHandler

BOOT_STACK_TOP  EQU     0x2407FFF0                          ; D1 SRAM 顶端 (默认可访问, 不需初始化)

                EXPORT  ResetHndlr

;******************************************************************************
;  Macro: fill one IRQ vector entry with SpuriousHandler_ISR
;******************************************************************************
                MACRO
                IRQ_ENTRY  $name
                DCD       SpuriousHandler_ISR
                MEND

;******************************************************************************
;  Vector table (必须在 offset 0x00 开始!)
;    Cortex-M 复位时从 0x00 取 initial SP, 从 0x04 取 Reset_Handler 地址
;    [0]   Initial SP
;    [1]   Reset
;    [2]   NMI
;    [3-6] Faults
;    [7-10] Reserved (must be 0)
;    [11]  SVCall
;    [12]  Debug Monitor
;    [13]  Reserved
;    [14]  PendSV
;    [15]  SysTick
;    [16-183] 168 IRQs (H743: IRQ0..IRQ167)
;******************************************************************************
Vectors
        DCD     BOOT_STACK_TOP                              ;  0, Initial SP
        DCD     ResetHndlr                                  ;  1, Reset
        DCD     SpuriousHandler_ISR                         ;  2, NMI
        DCD     SpuriousHandler_ISR                         ;  3, HardFault
        DCD     SpuriousHandler_ISR                         ;  4, MemManage
        DCD     SpuriousHandler_ISR                         ;  5, BusFault
        DCD     SpuriousHandler_ISR                         ;  6, UsageFault
        DCD     0                                           ;  7, Reserved
        DCD     0                                           ;  8, Reserved
        DCD     0                                           ;  9, Reserved
        DCD     0                                           ; 10, Reserved
        DCD     SpuriousHandler_ISR                         ; 11, SVCall
        DCD     SpuriousHandler_ISR                         ; 12, DebugMon
        DCD     0                                           ; 13, Reserved
        DCD     SpuriousHandler_ISR                         ; 14, PendSV
        DCD     SpuriousHandler_ISR                         ; 15, SysTick

        IRQ_ENTRY IRQ0
        IRQ_ENTRY IRQ1
        IRQ_ENTRY IRQ2
        IRQ_ENTRY IRQ3
        IRQ_ENTRY IRQ4
        IRQ_ENTRY IRQ5
        IRQ_ENTRY IRQ6
        IRQ_ENTRY IRQ7
        IRQ_ENTRY IRQ8
        IRQ_ENTRY IRQ9
        IRQ_ENTRY IRQ10
        IRQ_ENTRY IRQ11
        IRQ_ENTRY IRQ12
        IRQ_ENTRY IRQ13
        IRQ_ENTRY IRQ14
        IRQ_ENTRY IRQ15
        IRQ_ENTRY IRQ16
        IRQ_ENTRY IRQ17
        IRQ_ENTRY IRQ18
        IRQ_ENTRY IRQ19
        IRQ_ENTRY IRQ20
        IRQ_ENTRY IRQ21
        IRQ_ENTRY IRQ22
        IRQ_ENTRY IRQ23
        IRQ_ENTRY IRQ24
        IRQ_ENTRY IRQ25
        IRQ_ENTRY IRQ26
        IRQ_ENTRY IRQ27
        IRQ_ENTRY IRQ28
        IRQ_ENTRY IRQ29
        IRQ_ENTRY IRQ30
        IRQ_ENTRY IRQ31
        IRQ_ENTRY IRQ32
        IRQ_ENTRY IRQ33
        IRQ_ENTRY IRQ34
        IRQ_ENTRY IRQ35
        IRQ_ENTRY IRQ36
        IRQ_ENTRY IRQ37
        IRQ_ENTRY IRQ38
; Modify 2026.6.24 v23: BL 加 USART3_RXNE 中断 (ring buffer 用), 注册到向量 IRQ39 (USART3_IRQn)
        DCD     USART3_IRQHandler
        IRQ_ENTRY IRQ40
        IRQ_ENTRY IRQ40
        IRQ_ENTRY IRQ41
        IRQ_ENTRY IRQ42
        IRQ_ENTRY IRQ43
        IRQ_ENTRY IRQ44
        IRQ_ENTRY IRQ45
        IRQ_ENTRY IRQ46
        IRQ_ENTRY IRQ47
        IRQ_ENTRY IRQ48
        IRQ_ENTRY IRQ49
        IRQ_ENTRY IRQ50
        IRQ_ENTRY IRQ51
        IRQ_ENTRY IRQ52
        IRQ_ENTRY IRQ53
        IRQ_ENTRY IRQ54
        IRQ_ENTRY IRQ55
        IRQ_ENTRY IRQ56
        IRQ_ENTRY IRQ57
        IRQ_ENTRY IRQ58
        IRQ_ENTRY IRQ59
        IRQ_ENTRY IRQ60
        IRQ_ENTRY IRQ61
        IRQ_ENTRY IRQ62
        IRQ_ENTRY IRQ63
        IRQ_ENTRY IRQ64
        IRQ_ENTRY IRQ65
        IRQ_ENTRY IRQ66
        IRQ_ENTRY IRQ67
        IRQ_ENTRY IRQ68
        IRQ_ENTRY IRQ69
        IRQ_ENTRY IRQ70
        IRQ_ENTRY IRQ71
        IRQ_ENTRY IRQ72
        IRQ_ENTRY IRQ73
        IRQ_ENTRY IRQ74
        IRQ_ENTRY IRQ75
        IRQ_ENTRY IRQ76
        IRQ_ENTRY IRQ77
        IRQ_ENTRY IRQ78
        IRQ_ENTRY IRQ79
        IRQ_ENTRY IRQ80
        IRQ_ENTRY IRQ81
        IRQ_ENTRY IRQ82
        IRQ_ENTRY IRQ83
        IRQ_ENTRY IRQ84
        IRQ_ENTRY IRQ85
        IRQ_ENTRY IRQ86
        IRQ_ENTRY IRQ87
        IRQ_ENTRY IRQ88
        IRQ_ENTRY IRQ89
        IRQ_ENTRY IRQ90
        IRQ_ENTRY IRQ91
        IRQ_ENTRY IRQ92
        IRQ_ENTRY IRQ93
        IRQ_ENTRY IRQ94
        IRQ_ENTRY IRQ95
        IRQ_ENTRY IRQ96
        IRQ_ENTRY IRQ97
        IRQ_ENTRY IRQ98
        IRQ_ENTRY IRQ99
        IRQ_ENTRY IRQ100
        IRQ_ENTRY IRQ101
        IRQ_ENTRY IRQ102
        IRQ_ENTRY IRQ103
        IRQ_ENTRY IRQ104
        IRQ_ENTRY IRQ105
        IRQ_ENTRY IRQ106
        IRQ_ENTRY IRQ107
        IRQ_ENTRY IRQ108
        IRQ_ENTRY IRQ109
        IRQ_ENTRY IRQ110
        IRQ_ENTRY IRQ111
        IRQ_ENTRY IRQ112
        IRQ_ENTRY IRQ113
        IRQ_ENTRY IRQ114
        IRQ_ENTRY IRQ115
        IRQ_ENTRY IRQ116
        IRQ_ENTRY IRQ117
        IRQ_ENTRY IRQ118
        IRQ_ENTRY IRQ119
        IRQ_ENTRY IRQ120
        IRQ_ENTRY IRQ121
        IRQ_ENTRY IRQ122
        IRQ_ENTRY IRQ123
        IRQ_ENTRY IRQ124
        IRQ_ENTRY IRQ125
        IRQ_ENTRY IRQ126
        IRQ_ENTRY IRQ127
        IRQ_ENTRY IRQ128
        IRQ_ENTRY IRQ129
        IRQ_ENTRY IRQ130
        IRQ_ENTRY IRQ131
        IRQ_ENTRY IRQ132
        IRQ_ENTRY IRQ133
        IRQ_ENTRY IRQ134
        IRQ_ENTRY IRQ135
        IRQ_ENTRY IRQ136
        IRQ_ENTRY IRQ137
        IRQ_ENTRY IRQ138
        IRQ_ENTRY IRQ139
        IRQ_ENTRY IRQ140
        IRQ_ENTRY IRQ141
        IRQ_ENTRY IRQ142
        IRQ_ENTRY IRQ143
        IRQ_ENTRY IRQ144
        IRQ_ENTRY IRQ145
        IRQ_ENTRY IRQ146
        IRQ_ENTRY IRQ147
        IRQ_ENTRY IRQ148
        IRQ_ENTRY IRQ149
        IRQ_ENTRY IRQ150
        IRQ_ENTRY IRQ151
        IRQ_ENTRY IRQ152
        IRQ_ENTRY IRQ153
        IRQ_ENTRY IRQ154
        IRQ_ENTRY IRQ155
        IRQ_ENTRY IRQ156
        IRQ_ENTRY IRQ157
        IRQ_ENTRY IRQ158
        IRQ_ENTRY IRQ159
        IRQ_ENTRY IRQ160
        IRQ_ENTRY IRQ161
        IRQ_ENTRY IRQ162
        IRQ_ENTRY IRQ163
        IRQ_ENTRY IRQ164
        IRQ_ENTRY IRQ165
        IRQ_ENTRY IRQ166
        IRQ_ENTRY IRQ167

__Vectors_End
        ALIGN

;******************************************************************************
;  Reset_Handler (在 Vectors 表之后, 由 Vectors[1] 指向)
;******************************************************************************
ResetHndlr       PROC
                LDR     R0, =BOOT_STACK_TOP
                MSR     MSP, R0
                LDR     R0, =Bootloader_Main
                BX      R0
                ENDP

                END
