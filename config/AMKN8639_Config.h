/************************************************************************************
*  Copyright (C) 2004-2026, zqly. All rights reserved.
*  www.embedarm.com / embedarm@126.com
*
*  File name  : AMKN8639_Config.h
*  Project    : STM32 / GD32 ÏµÁÐÄ£°å¹¤³Ì
*  Processor  : STM32H743 / GD32F450 µÈ
*  Compiler   : RealView MDK-ARM Compiler (ARMCC v5)
*
*  Description: AMKN8639 °å¼¶Ó²¼þÅäÖÃÍ·ÎÄ¼þ.
*                ±¾ÎÄ¼þ¼¯ÖÐ¶¨ÒåÈ«²¿¹¦ÄÜ¿ª¹Ø (Feature Flags)¡¢Ó²¼þ²ÎÊý (Hardware
*                Parameters) Óë¿Éµ÷ãÐÖµ (Tunables), Í¨¹ýÐÞ¸Ä±¾ÎÄ¼þ¼´¿É¿ìËÙÊÊÅä
*                ²»Í¬ÏîÄ¿, ²»±Ø¸Ä¶¯¿â´úÂë.
*
*  ±àÂë       : GB2312 (¼òÌåÖÐÎÄ, Ò»¼¶ + ¶þ¼¶ºº×Ö + ASCII).
*                ËùÓÐÖÐÎÄ×¢ÊÍÑÏ¸ñÊ¹ÓÃ GB2312 ÊÕÂ¼×Ö·û, ²»º¬ GB18030 À©Õ¹×Ö.
*
*  ÐÞ¸Ä¼ÇÂ¼   :
*    V1.30  2026-07-02  GB2312 ÖØÐ´, ¹Ø±Õ LVGL/LCD/LTDC/DMA2D/JPEG, ½ÚÊ¡Ô¼ 1MB
*    V1.20  2022-01-01  ÕûÀí²ÎÊý, ÕýÊ½·¢²¼ V1.20
*    V1.10  2021-09-01  ÐÞ¸Ä DMA ÅäÖÃ
*
*  Ê¹ÓÃ×¢Òâ   :
*    1) ÐÞ¸Ä¿ª¹Øºó±ØÐëÖØÐÂ±àÒëÕû¹¤³Ì (Build All);
*    2) ¹Ø±ÕÄ³ÈÎÎñºó, ÈôÈÔÒýÓÃÆäÈ«¾Ö±äÁ¿, ÐèÔÚ¶ÔÓ¦ .c ÎÄ¼þÖÐÍ¬²½°ü #if;
*    3) Flash ÉÕÂ¼Í¨¹ý STM32_Programmer_CLI Íê³É, ¼û
*       targets\\AMKN8639\\BL_BUILD\\scripts\\ota_host.py.
************************************************************************************/

/*
*  Copyright (c), 2004-2026, ±±¾©ÖÐÇ¶ÁèÔÆµç×ÓÓÐÏÞ¹«Ë¾
*            All rights reserved.
*
* http:  www.embedarm.com
* Email: embedarm@126.com
*
* File name: AMKN8639_Config.h
* Project  : STM/GD32F1XX/3XX/4XX/H7XXÏµÁÐÄ£¿éÈí¼þ
* Processor: STM/GD32F1XX/3XX/4XX/H7XX
* Compiler : RealView MDK-ARM Compiler
*
* Author:  EmbedARM
* Email:   EmbedARM@126.com
*
* Description: ±¾ÎÄ¼þÊÇAMKN8639°å¼¶Ó²¼þÅäÖÃÎÄ¼þ, ¶¨Òå¹¦ÄÜ¿ª¹ØºÍ²ÎÊý;
*
* Others: none;
*
* Function List:
*
* Modifiy History:
*   1. Version: 1.30
*      Date:    2026.7.2
*      Modifiy: GB2312 ÖØÐ´, ÐÞÕýÔ­ GBK ±àÂë´íÂÒ
*
*   2. Version: 1.20
*      Date:    2022.1.1
*      Modifiy: ÕýÊ½·¢²¼ V1.20
*
*   3. Version: 1.10
*      Date:    2021.9.1
*      Modifiy: ÐÞ¸Ä DMA ²¿·Ö
*/
#ifndef __AMKN8639_CONFIG_H 
#define __AMKN8639_CONFIG_H  // __AMKN8639_CONFIG_H ÅäÖÃ²ÎÊý


/************************************************************************************/
// ·é¤æ§£¨å¤´¾¥ç¡·æ
/************************************************************************************/
#include "const.h" 
#include "AMKN.h"  

/************************************************************************************/
// AT¤æ·é¤æ  
/************************************************************************************/
#define AT_EN          1            // AT¤æä½¿é¤æ: 0, ; 1, ä½¿é¤æ;

/************************************************************************************/
// ·é¥ç¹æ·é¤æ·é¤æ, ä¼¼´©æ·å·é¤æ·é¤æ·è·é¤æ  
/************************************************************************************/
#define PRODUCT_MODEL  "AMKN8639"             // ·å´æ, ä¼¼´©æ·å·é¤æè¦©æ·é¤æ·ç
#define PRODUCT_NAME   "AMKN8639"                    // äº§å§°, ¨æ¹æå®è¦¿®¹èä¸å­¸²

#define HW_VERSION         100                // ç¡·é¥æV1.10
/************************************************************************************/
// ç³»ç·é¤æ·é¤æ¤æ 
// ä¼·é¤æ·é¤æ·é¤æ·é1·ä½¿©æ·é¤æ·éï½·é0·é©æ·é¤æ·é¤æ
/************************************************************************************/
#define TASK_FILE_EN      	1	  // FILE·é¤æ·é¤æ         0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_LWIP_EN      	1	  // LWIP(TCPIP)·é¤æ·é¤æ  0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_EVENTCTRL_EN   0	  // ç¡·æ·é¤æ·é¤æ           0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_ZQXYCTRL_EN    0	  // ·åä»¤æ·é¤æ       0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_USERCTRL_EN    0	  // ä¼·é¤æ·é¤æ         0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_MODBUS_EN      0	  // MODBUS·é¤æ·é¤æ·é¤æ   0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_CANOPEN_EN     0	  // CANOPEN·é¤æ·é¤æ·é¤æ  0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_LCD_EN         0	  // LCD·ç¤º·é¤æ          0, ; 1, ä½¿é¤æ·é¤æ;
#define TASK_TEST_EN        1	  // ·é¤æ·é¤æ             0, ; 1, ä½¿é¤æ·é¤æ;

/************************************************************************************/
// ç³»ç¶é¥è¹æ³»ç»¤æ¶é¤æ·é¤æ  
// æ³¨éè§£ïSYSCLK_HSE·é¤æ·é¤æ50MHZ, ·éç¼´é·é¤æ; SYSTIM_TICK_T·é¤æ·é¤æä¸1·éç¼´é·é¤æ
//       SYSCLK·é¤æ·é¤æä¸: SYSCLK_50MHZ,SYSCLK_100MHZ,SYSCLK_200MHZ,SYSCLK_300MHZ
//                         SYSCLK_400MHZ,SYSCLK_480MHZ
/************************************************************************************/
#define SYSCLK_HSE       50000000           // ·é¤æ§£¨æ·æ, ·é¤æ·ç·é
#define SYSCLK           SYSCLK_480MHZ      // ç³»ç¶é¤æ

#define SYSTIM_TICK_T    1      // ç³»ç·æ·é¤æ¶æ, ·äms, ·é¤æ·ç·é

#define SYS_ICACHE_EN    1      // ¤æCacheä½¿é¤æ: 1,ä½¿é¤æ; 0, 
#define SYS_DCACHE_EN    1      // ·é¤æCacheä½¿é¤æ: 1,ä½¿é¤æ; 0, 
/************************************************************************************/
// ·é¤æMCU·é¡æ¾å·é¤æ
/************************************************************************************/
#define  MCU_IDLE_EN        1        // 0, ç¡·æCU·é¡æ¾å; 1, ä½¿é°ç¡··é¤æMCU·é¡æ¾å
#define  MCU_IDLE_SCAN_T    1000     // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
     
/************************************************************************************/
// è¯§æ·é¤æ·é·é¤æ·é¤æ 
/************************************************************************************/
#define IWDG_EN        0          // è¯§æ·é·ä½¿, 1·é¤æä½¿é°ï 0·é
#define IWDG_TIME      3000       // ·é·æ·é, , ·é¤æ:200~26000ms(·é¤æ1.0426)

/************************************************************************************/
//  DI·é¤æ·é¤æ·é
/************************************************************************************/
#define DI_EN           1           // DIä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define DI_MODE         0           // ·é¤æDI·é¤ææ¨¡å: 0, å®ç¢·é¤æ·å+·è¢é¤æ¨¡å¼æ¨¡å; 1, ·è¢é¤æ¨¡å¼; 
#define DI_SCAN_T       10          // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#define DI_ATOUT_T      3000        // ·é¤æè®¹æ·éç»·é¤æ, ·ä: ms

/************************************************************************************/
//  SW·é¤æ·é¤æ·é
/************************************************************************************/
#define SW_EN           1           // SWä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define SW_MODE         0           // ·é¤æSW·é¤ææ¨¡å: 0, å®ç¢·é¤æ·å+·è¢é¤æ¨¡å¼æ¨¡å; 1, ·è¢é¤æ¨¡å¼; 
#define SW_SCAN_T       100         // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#define SW_ATOUT_T      3000        // ·é¤æè®¹æ·éç»·é¤æ, ·ä: ms

/************************************************************************************/
//  KEY·é¤æ·é¤æ
/************************************************************************************/
#define KEY_EN           1           // KEYä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define KEY_MODE         0           // ·é¤æKEY·é¤ææ¨¡å: 0, å®ç¢·é¤æ·å+·è¢é¤æ¨¡å¼æ¨¡å; 1, ·è¢é¤æ¨¡å¼; 
#define KEY_SCAN_T       10          // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#define KEY_CDOWN_T      (KEY_SCAN_T*100)  // ·éå¸·é¤æ·é¤æ·é¤æ·éç»·é¤æ, ·ä: ms; æ³¨é¤æ: ·é¤æ¹æ¶æ·æ·é¤æ·é¤æ, ·é¤æä¸0·ç¤º·é¤æ

/************************************************************************************/
//  DO·é¤æ·é¤æ·é
/************************************************************************************/
// DO_EN·é¤æ·é¤æä¸1, ·é¤æ·é
#define DO_EN           1           // DOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define DO_SCAN_T       1           // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

/************************************************************************************/
// EXTI0~EXTI19 ·é¤æè®¹æ·é¤æ
/************************************************************************************/
// EXTI0è®¹æ·é¤æ
#define EXTI0_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI0_IO        PF0      // A0, PB0, PC0, PD0, PE0, PF0, PG0, PH0, PI0·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI0_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI1è®¹æ·é¤æ
#define EXTI1_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI1_IO        PF1      // A1, PB1, PC1, PD1, PE1, PF1, PG1, PH1, PI1·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI1_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI2è®¹æ·é¤æ
#define EXTI2_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI2_IO        PF2      // A2, PB2, PC2, PD2, PE2, PF2, PG2, PH2, PI2·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI2_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI3è®¹æ·é¤æ(CH455è®¹æ·é¤æ)
#define EXTI3_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI3_IO        PF3      // A3, PB3, PC3, PD3, PE3, PF3, PG3, PH3, PI3·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI3_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI4è®¹æ·é¤æ
#define EXTI4_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI4_IO        PF4      // A4, PB4, PC4, PD4, PE4, PF4, PG4, PH4, PI4 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI4_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI5è®¹æ·é¤æ
#define EXTI5_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI5_IO        PF5      // A5, PB5, PC5, PD5, PE5, PF5, PG5, PH5, PI5 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI5_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI6è®¹æ·é¤æ
#define EXTI6_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI6_IO        PF6      // A6, PB6, PC6, PD6, PE6, PF6, PG6, PH6, PI6 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI6_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI7è®¹æ·é¤æ
#define EXTI7_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI7_IO        PF7		 // A7, PB7, PC7, PD7, PE7, PF7, PG7, PH7, PI7 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI7_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI8è®¹æ·é¤æ
#define EXTI8_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI8_IO        PF8	     // A8, PB8, PC8, PD8, PE8, PF8, PG8, PH8, PI8 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI8_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI9è®¹æ·é¤æ
#define EXTI9_EN        0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI9_IO        PF9	     // A9, PB9, PC9, PD9, PE9, PF9, PG9, PH9, PI9 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI9_MODE      0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI10è®¹æ·é¤æ
#define EXTI10_EN       0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI10_IO       PF10     // A10, PB10, PC10, PD10, PE10, PF10, PG10, PH10, PI10 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI10_MODE     0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI11è®¹æ·é¤æ
#define EXTI11_EN       1		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI11_IO       PD11     // A11, PB11, PC11, PD11, PE11, PF11, PG11, PH11, PI11 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI11_MODE     1		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI12è®¹æ·é¤æ
#define EXTI12_EN       0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI12_IO       PF12     // A12, PB12, PC12, PD12, PE12, PF12, PG12, PH12, PI12 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI12_MODE     0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI13è®¹æ·é¤æ
#define EXTI13_EN       0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI13_IO       PF13     // A13, PB13, PC13, PD13, PE13, PF13, PG13, PH13, PI13 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI13_MODE     0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI14è®¹æ·é¤æ
#define EXTI14_EN       0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI14_IO       PF14     // A14, PB14, PC14, PD14, PE14, PF14, PG14, PH14, PI14 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ;
#define EXTI14_MODE     0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI15è®¹æ·é¤æ
#define EXTI15_EN       0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI15_IO       PF15     // A15, PB15, PC15, PD15, PE15, PF15, PG15, PH15, PI15 ·é¤æä¸O·ä¸ºè®¹æ·é¤æ; 
#define EXTI15_MODE     0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI16_PVDè®¹æ·é¤æ(EXIT16·é¥çPVD·é)
#define EXTI16_PVD_EN               0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI16_PVD_MODE             0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI17_RTCAlarmè®¹æ·é¤æ(EXIT17·é¥çRTC·é¤æç¡·æ)
#define EXTI17_RTCAlarm_EN          0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
#define EXTI17_RTCAlarm_MODE        0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI18_USBWakeUpè®¹æ·é¤æ(EXIT18·é¥çUSB·é¤æç¡·æ)
//#define EXTI18_USBWakeUp_EN         0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
//#define EXTI18_USBWakeUp_MODE       0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;


// EXTI19_NETWakeUpè®¹æ·é¤æ(EXIT18·é¥ç·å¤ª·é¤æ·é°ç¡·)
//#define EXTI19_NETWakeUp_EN         0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
//#define EXTI19_NETWakeUp_MODE       0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI20_USBHSWakeUpè®¹æ·é¤æ(EXIT20·é¥çUSB HOST·é¤æç¡·æ)
//#define EXTI20_USBHSWakeUp_EN         0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
//#define EXTI20_USBHSWakeUp_MODE       0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI21_RTCTSEè®¹æ·é¤æ(EXIT21·é¥çRTC·é¤æè®¹æ)
//#define EXTI21_RTCTSE_EN         0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
//#define EXTI21_RTCTSE_MODE       0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI22_RTCWakeUpè®¹æ·é¤æ(EXIT22·é¥çRTC·éè¡·æ·é¤æ¸é)
//#define EXTI22_RTCWakeUp_EN         0		 // ä¼ç¡·æä½¿é¤æ: 0,´æç¡·æ; 1, ·éè¯§æç­¹æå§¤æ·ä½¿; 2, ·é°ç¡··é¤æ·é¹æå§¤æ·ä½¿; 3, ·éè¯§æç­¹æå§¤æè¯§æä½¿é¤æ; 4, ·é°ç¡··é¤æ·é¹æå§¤æè¯§æä½¿é¤æ; 
//#define EXTI22_RTCWakeUp_MODE       0		 // ·é¤æ´æç¡·ææ¨¡å: 0,·é¤æè¾¾æ·é´æç¡·æ; 1,¤æè¾¾æ·é´æç¡·æ; 2,·é¤æ·é°æ·éè®¹æ·é¤æ´æç¡·æ;

// EXTI ×ÜÊ¹ÄÜ (ÈÎÒâ EXTI ÆôÓÃÔò¿ªÆô)
#define EXTI_EN  (EXTI0_EN+EXTI1_EN+EXTI2_EN+EXTI3_EN+EXTI4_EN+EXTI5_EN+EXTI6_EN+EXTI7_EN+EXTI8_EN+EXTI9_EN+EXTI10_EN \
                  +EXTI11_EN+EXTI12_EN+EXTI13_EN+EXTI14_EN+EXTI15_EN+EXTI16_PVD_EN+EXTI17_RTCAlarm_EN)
/************************************************************************************/
// UART1 ·é¤æ·é¤æ
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART1_EN          1       // UART1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART1_EN > 0)
#define UART1_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART1_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus·é¤æ·é¤æ
      
#define UART1_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART1_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART1_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART1_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART1_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART1_FIFO_EN     1        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;  ·é¤æä½¿é¤æ;
#if (UART1_FIFO_EN > 0)
#define UART1_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART1TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;   
#define UART1RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART1_RXBUF_SIZE  4096    /* was 256, increased for FWDL OTA */     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART1_TXBUF_SIZE  1024     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif
/************************************************************************************/
// UART2 ·é¤æ·é¤æ 
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART2_EN          1       // UART2ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART2_EN > 0)
#define UART2_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART2_RX_EN       0       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus
#define UART2_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART2_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART2_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART2_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART2_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART2_FIFO_EN     1       // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;  
#if (UART2_FIFO_EN > 0)
#define UART2_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART2TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART2RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART2_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ,·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART2_TXBUF_SIZE  256     // ·é¤æä¼¿é¤æ,·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif
/************************************************************************************/
// UART3 ·é¤æ·é¤æ 
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART3_EN          1      // UART3ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART3_EN > 0)
#define UART3_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART3_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus
                                  
#define UART3_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART3_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART3_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART3_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART3_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART3_FIFO_EN     1        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;  
#if (UART3_FIFO_EN > 0)
#define UART3_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART3TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART3RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART3_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART3_TXBUF_SIZE  256     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif
/************************************************************************************/
// UART4 ·é¤æ·é¤æ
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART4_EN          1       // UART4ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART4_EN > 0)
#define UART4_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART4_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus
                                  
#define UART4_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART4_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART4_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART4_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART4_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART4_FIFO_EN     1        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;  
#if (UART4_FIFO_EN > 0)
#define UART4_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART4TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART4RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART4_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART4_TXBUF_SIZE  256     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª;  

#endif
/************************************************************************************/
// UART5 ·é¤æ·é¤æ 
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART5_EN          0      // UART5ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART5_EN > 0)
#define UART5_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART5_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus
                                  
#define UART5_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART5_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART5_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART5_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART5_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART5_FIFO_EN     0        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;  
#if (UART5_FIFO_EN > 0)
#define UART5_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART5TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART5RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART5_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART5_TXBUF_SIZE  256     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif
/************************************************************************************/
// UART6 ·é¤æ·é¤æ 
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART6_EN          1      // UART6ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART6_EN > 0)
#define UART6_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART6_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus
                                  
#define UART6_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART6_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART6_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART6_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART6_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART6_FIFO_EN     1        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é;  
#if (UART6_FIFO_EN > 0)
#define UART6_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART6TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART6RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART6_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART6_TXBUF_SIZE  256     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif

/************************************************************************************/
// UART7 ·é¤æ·é¤æ 
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART7_EN          1      // UART7ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART7_EN > 0)
#define UART7_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART7_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus·é¤æ·é¤æ
                                  
#define UART7_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART7_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART7_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART7_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART7_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART7_FIFO_EN     1        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#if (UART7_FIFO_EN > 0)
#define UART7_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART7TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART7RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART7_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART7_TXBUF_SIZE  2048     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif

/************************************************************************************/
// UART8 ·é¤æ·é¤æ 
// æ³¨é¤æ1·é¤æ·é¤æ·é¤æç¡·æ·éè½¿ä·é¤æ¹æ·éç­¹æ9bit·é¤æä¼it·é¤æ¶é¤æ·ä
// æ³¨é¤æ2·é¤æå°¤æ¤æIFOä½¿é¤æ; ·é¤æMAä½¿é¤æ, ¤æ·é¤æç­¹æ¥ºè¾¾æ16è¯§æ·é¤æMA, ·é¤æ·é¤æ;
/************************************************************************************/
#define UART8_EN          1      // UART8ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (UART8_EN > 0)
#define UART8_RXMODE      0       // ·é¤æ·é·ç·é¤ææ¨¡å·é¤æ: 0, UART_RXMODE_SCAN; 1, UART_RXMODE_IRQ; 2, UART_RXMODE_ISRHOOK;
#define UART8_RX_EN       1       // ·é¤æ·é¤æä½¿é¤æ: 1·é¤æä½¿é¤æ; 0, ;  
                                  // æ³¨é¤æ: ·é¤æAARTåº¤æodbus·é¤æ¤æ(IFI¤æ), ·é¤æ·é¤æä¸0, ¡æç¢ç¢odbus·é¤æ·é¤æ
                                  
#define UART8_SCAN_T      10      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define UART8_BAUD        115200  // ·éè¯§æ·éç»·é¤æ·é¤æï½1200240048009600192003840057600115200
#define UART8_WORD_LENGTH 0       // ·é¤æ·é¤æ¹æ,   0: 8bit;   1: 9bit;
#define UART8_STOP_BITS   0       // ·é¤æä½,     0: 1bit;   1: 2bit;    2: 0.5bit;  3: 1.5bit;
#define UART8_PARITY      0       // ·é¤æ·å·é¤æä½, 0: ·æ; 1: ¶æ;  2: ·æ;

#define UART8_FIFO_EN     1        // ·é¤æ¤æIFOä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#if (UART8_FIFO_EN > 0)
#define UART8_RX_TIMEOUT  30      // ·é¤æ·ç, ·äs;
#endif

#define UART8TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define UART8RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#define UART8_RXBUF_SIZE  256     // ·é¤æ§¸ä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 
#define UART8_TXBUF_SIZE  256     // ·é¤æä¼¿é¤æ, ·å·é¤æ0, ·é¤æç¡·æå®¤æ·è¤æ, ·é¤æ·å¤ª; 

#endif

/************************************************************************************/
// EEPROM ·é¤æ·é¤æ(·é¤æ·é§æä½¿é¤æI2C1·é¤æ)
// ·é¤æ·é¤æä¸¤æ
/************************************************************************************/
#define EEPROM_EN          1          // EEPROMä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define EEPROM_DEVICE      AT24C64    // ·é¤æ·é¤æ´æ	
#define EEPROM_FREQ        400000     // ·å¶é¤æé¢¤æ

#if (EEPROM_EN == 0)
  #error "ERROR: EEPROM_EN ±ØÐëÏÈÊ¹ÄÜ!"
#endif
 
/************************************************************************************/
// SPI1·é¤æ·éï½
// SPI1¶é¤æé¢¤æ=64MHZ/SPI1_DIVCLK, ·éçµI1_DIVCLKPI_DIVCLK_16, ·æ·é¤æ4MHZ
/************************************************************************************/
#define SPI1_EN          0                // SPIä½¿é¤æ,      1·é¤æä½¿é°ï 0·é

#if (SPI1_EN > 0)

#define SPI1_CKMODE      SPI_CKMODE3        // ¶é¤æ·äæ¨¡å, ½¿ç¡·æspi.h·è´é¤æ 
#define SPI1_DIVCLK      SPI_DIVCLK_16      // SPI¶é¥å·é³»

//SPI DMA·é¤æ
#define SPI1TX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#define SPI1RX_DMA_EN    0        // ·é¤æMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 

#endif

/************************************************************************************/
// QSPI·é¤æ·éï½
// QSPI¶é¤æé¢¤æ=SYSCLK/2/QSPI_DIVCLK, ·éçµPI_DIVCLK4,SYSCLK480MHZ, ·æ·é¤æ60MHZ
/************************************************************************************/
#define QSPI_EN          1                // QSPIä½¿é¤æ,      1·é¤æä½¿é°ï 0·é

#if (QSPI_EN > 0)  
#define QSPI_DEVICE  GD25Q64                  // ·é¤æ¤æ
#define QSPI_DIVCLK  QSPI_DIVCLK_4            // QSPI¶é¥å·é³», Modify 2025.10.12 ·ä¸º4·éé¥ºæ·éé¥ºè¹æ
#define QSPI_IOMODE  QSPI_IOMODE_2_LINES      // IO·é¤æ·éè®¹æ·é¤æ·é¤æç­
#define QSPI_CKMODE  QSPI_CLK_MODE0           // CLKæ¨¡å·æ¨¡å¼0SPI_CKMODE0 æ¨¡å3SPI_CKMODE3
#define QSPI_FLASH_SIZE QSPI_FLASH_SIZE_8MB   // Flash·å·é è®¹æ8MB
#endif

/************************************************************************************/
// RTC ·é¤æ·é¤æ 
/************************************************************************************/
#define RTC_EN           1         // RTCä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define RTC_SCAN_T       1000      // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define RTC_WKUP_EN      0         // RTC·é¤æä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	
#define RTC_WKUPIT_EN    0         // RTC·é¤æè®¹æä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	

#define RTC_ALRA_EN      0         // RTC·é¤æAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	
#define RTC_ALRAIT_EN    0         // RTC·é¤æAè®¹æä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	

#define RTC_ALRB_EN      0         // RTC·é¤æBä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	
#define RTC_ALRBIT_EN    0         // RTC·é¤æBè®¹æä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	

#define RTC_TSF_EN       0         // RTC¶é¤æ¸æ, 1·é¤æä½¿é°ï 0·é
#define RTC_TSFIT_EN     0         // RTC¶é¤æ·å¸æ, 1·é¤æä½¿é°ï 0·é

#define RTC_TAMP1_EN     0         // RTC·éè¡¡··é1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define RTC_TAMP1IT_EN   0         // RTC·éè¡¡··é1è®¹æä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define RTC_TAMP2_EN     0         // RTC·éè¡¡··é1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	
#define RTC_TAMP2IT_EN   0         // RTC·éè¡¡··é1è®¹æä½¿é¤æ, 1·é¤æä½¿é°ï 0·é	

// ç¢¾¥ç­¹æå§¤æ¶é: 
#define RTC_YEAR         22        // ·å¤æï¼22 ·ç¤º2022
#define RTC_MONTH        4         // ·å¤æï½4  ·ç¤º4¤æ
#define RTC_DAY          30        // ·å¤æï¼30 ·ç¤º30
#define RTC_HOUR         23        // ·å¤æå°23 ·ç¤º23
#define RTC_MINUTE       59        // ·å¤æ·é¥ï59 ·ç¤º59
#define RTC_SECOND       30        // ·å¤æï¼30 ·ç¤º30
/************************************************************************************/
// BKP ·é¤æ·é¤æ, ·é¤æ·é¤æ·ä¸ºä½¿é¤æ, ä¼·é©æ
/************************************************************************************/
#define BKP_EN          1          // BKPä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

/************************************************************************************/
//  CAN1, CAN2 ¤æ·é¤æ
//  æ³¨é¤æ: CANå¸§æ¨¡å¼¤æ2(ä¼·é¤æ), ·é¤æCAN1/2_BAUD·é¤æ200000, CAN1/2_DATA_BAUD·é¤æ400000¶é¤æ
/************************************************************************************/
// CAN1·é¤æ
#define CAN1_EN          1		       // CAN1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (CAN1_EN > 0)
#define CAN1_MODE		 0		       // 0(CAN_MODE_NORMAL),·é¤ææ¨¡å; 1(CAN_MODE_RESTRICTED_OPERATION), ·éè¯§æ·æ¨¡å¼; 2(CAN_MODE_BUS_MONITORING), ·éç«ç¡·æ¾¥ï¼; 
                                       // 3(CAN_MODE_INTERNAL_LOOPBACK), è¯§æ·é¤ææ¨¡å(·é); 4(CAN_MODE_EXTERNAL_LOOPBACK), §£¨é¤æ·æ¨¡å¼(·é)

#define CAN1_RXMODE      0             // ·é¤æ·æ·é¤ææ¨¡å·é¤æ: 0, CAN_RXMODE_SCAN; 1, CAN_RXMODE_IRQ; 
#define CAN1_SCAN_T      1             // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define CAN1_IDE         CAN_EXT_ID    // å¸§é¤æ: 0, ·åå¸:CAN_STD_ID; 1, ·åå¸:CAN_EXT_ID;
#define CAN1_RTR         CAN_RTR_DATA  // ¤æ·é¤æå¸: 0, CAN_RTR_DATA; è¿¤æå¸: 1, CAN_RTR_REMOTE;
#define CAN1_BAUD	     1000000       // CAN1¤æ·é¤æ; 
#define CAN1_DATA_BAUD 	 (2*CAN1_BAUD) // CAN1·é·è§æ·é¤æ, æ³¨é¤æ·é¤æ·é¤æ·éç´Nå¸§æ¨¡å¼¤æCAN_FRAME_FD_BRS(ä¼·é¤æ)æ¨¡å·é¤æ
                                       // ·é¤æè¯§æ·éç»·é¤æ·é¤æ·é¤æ1-5,¤æ·é¤æ2           

#define CAN1_FRAME_FORMAT  0           // CANå¸§æ¨¡å¼: 0, CAN_FRAME_CLASSIC:CAN·åæ¨¡å
                                       //            1, CAN_FRAME_FD_NO_BRS:FDCANæ¨¡å·é¤æ·éä¼·é¤æ; 
                                       //            2, CAN_FRAME_FD_BRS:FDCANæ¨¡å·éä¼·é¤æ: æ³¨é¤æ·æ·æ·é¤æ¾¥ï¼
                                       
#define CAN1_TXDATA_SIZE   8           // ·é¤æ·é¤æ·é: 8/12/16/20/24/32/48/64
#define CAN1_RXDATA_SIZE   8           // ·é¤æ·é¤æ·é: 8/12/16/20/24/32/48/64
#endif


// CAN2·é¤æ
#define CAN2_EN          1		       // CAN2ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (CAN2_EN > 0)
#define CAN2_MODE		 0		       // 0(CAN_MODE_NORMAL),·é¤ææ¨¡å; 1(CAN_MODE_RESTRICTED_OPERATION), ·éè¯§æ·æ¨¡å¼; 2(CAN_MODE_BUS_MONITORING), ·éç«ç¡·æ¾¥ï¼; 
                                       // 3(CAN_MODE_INTERNAL_LOOPBACK), è¯§æ·é¤ææ¨¡å(·é); 4(CAN_MODE_EXTERNAL_LOOPBACK), §£¨é¤æ·æ¨¡å¼(·é)

#define CAN2_RXMODE      0             // ·é¤æ·æ·é¤ææ¨¡å·é¤æ: 0, CAN_RXMODE_MT; 1, CAN_RXMODE_IRQ;
#define CAN2_SCAN_T      1             // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define CAN2_IDE         CAN_EXT_ID    // å¸§é¤æ: 0, ·åå¸:CAN_STD_ID; 1, ·åå¸:CAN_EXT_ID;
#define CAN2_RTR         CAN_RTR_DATA  // ¤æ·é¤æå¸: 0, CAN_RTR_DATA; è¿¤æå¸: 1, CAN_RTR_REMOTE;
#define CAN2_BAUD	     1000000       // CAN2·é¤æ; 
#define CAN2_DATA_BAUD 	 (2*CAN2_BAUD) // CAN2·é·è§æ·é¤æ, æ³¨é¤æ·é¤æ·é¤æ·éç´Nå¸§æ¨¡å¼¤æCAN_FRAME_FD_BRS(ä¼·é¤æ)æ¨¡å·é¤æ
                                       // ·é¤æè¯§æ·éç»·é¤æ·é¤æ·é¤æ1-5,¤æ·é¤æ2  
                                       
#define CAN2_FRAME_FORMAT  0           // CANå¸§æ¨¡å¼: 0, CAN_FRAME_CLASSIC:CAN·åæ¨¡å
                                       //            1, CAN_FRAME_FD_NO_BRS:FDCANæ¨¡å·é¤æ·éä¼·é¤æ;
                                       //            3, CAN_FRAME_FD_BRS:FDCANæ¨¡å·éä¼·é¤æ: æ³¨é¤æ·æ·æ·é¤æ¾¥ï¼
#define CAN2_TXDATA_SIZE   8           // ·é¤æ·é¤æ·é: 8/12/16/20/24/32/48/64
#define CAN2_RXDATA_SIZE   8           // ·é¤æ·é¤æ·é: 8/12/16/20/24/32/48/64
#endif


/***********************************************************************************
// ADC·é¤æ·é¤æ
*********************************************************************************/
#define ADC_EN         1      // ADCä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (ADC_EN > 0)

#define ADC_MODE       0      // 0, ADC_MODE_SWSTART: ¤æ¶é¤æ·é¤æ·é¤æ¶é¤æ·å°¾ä¾¥ï¼ª
                              // 1, ADC_MODE_EXTSEL: ¤æ§£¨é¤æ·éé¥ºç¡··é¤æä¾¥ï¼ª, ·ææ²¡é¤æå®¤æ

#define ADC_DOUBLE_BUFFER_EN  0   // ·é¤ææ¨¡å: 1, ä½¿é¤æ¤æ; 0, ä»ä¼ 
                                  // ä½¿é¤æ¤æ·å¨²¼´ç¡·æ·é¤æ, ç¡·æ·é¤æ·éé¥ºèè¾¾æ·é¤æ·é¤æ·é·è¾¾·é¤æ·éå¾¾¾·æ

#if (ADC_DOUBLE_BUFFER_EN > 0)
#define ADC_NOAVG_EN          0   // ·é¤æå¹³é¤æ·é¤æ: 1, è¯§æ·é¤æ·é¤æå¹³é¤æ·é¤æ, ç¼´ç¡··é¤æ·é¤æ; 0, è¯§æ·å¹³·é¤æ 
                                  // æ³¨éè§£ï·é¤æ·é¤ææ¨¡å·éè¯¥æ·å, ·é¤æ·æ
#endif                                  

// ·éè®¹æDè½·é¤æ
#define ADC_READ_MODE         0   // ·é¤æ¤æ: 0DC_MODE_IRQ,¤æè®¹æ·éç´·é¤æ;
                                  //           1DC_MODE_SCAN, ¤æ¶æ, DC_Read()·é¤æ·å·é¤æ
#define ADC_SCAN_T            100 // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

// æ¨¡é¤ææ¨¡é¤æ·é¤æ·ä½¿·é¤æ
#define AI1_EN        1      // AI1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI2_EN        1      // AI2ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI3_EN        1      // AI3ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI4_EN        1      // AI4ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI5_EN        1      // AI5ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI6_EN        1      // AI6ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI7_EN        1      // AI7ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI8_EN        1      // AI8ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define AI9_EN        0      // AI9ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é 
#define AI10_EN       0      // AI10ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

// æ¨¡é¤ææ¨¡é¤æ·é¤æ·é¤æ¸æ·é¤æ
#define AI1DIF_EN     0      // AI1·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI2DIF_EN     0      // AI2·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI3DIF_EN     0      // AI3·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI4DIF_EN     0      // AI4·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI5DIF_EN     0      // AI5·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI6DIF_EN     0      // AI6·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI7DIF_EN     0      // AI7·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI8DIF_EN     0      // AI8·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI9DIF_EN     0      // AI9·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ
#define AI10DIF_EN    0      // AI10·é¤æ·éç»·é, 1·ä½¿è¯§æ 0·é,·é¤æ·é¤æ

#define ADC_CHNUM      (AI1_EN+AI2_EN+AI3_EN+AI4_EN+AI5_EN+AI6_EN+AI7_EN+AI8_EN+AI9_EN+AI10_EN)	  // ·é¤æ¤æ
#define ADC_AVGNUM     4                    // ·éè¯§æ·é¤æ·é¤æ·é¤æå¹³é¤æ, ·å 1~256, æ³¨éè§£ï·å¼å¤ª·éç§¸ç¡··ç½é¤æè¯ç§¸ç¡·

#define ADC_SAMPLE_TIME  ADC_SAMPLE_10CLK   //ADC_SAMP7T0US      // ·é¤æ·é
#define ADC_FREQ       1                    // æ¯¤æ·é¤æ·é¤æ·é¤æ·é¤æ(·é¤æå¹³é¤æ¼é¤æ·é)

#define ADC_OVSR       ADC_OVSR_X32  // ¤æ·é¤æ·é: ADC_OVSR_X1, æ²¡é¸æ·é¤æ; ADC_OVSR_X2, 2·é¤æ·é¤æ; ADC_OVSR_X4, 4·é¤æ·é¤æ; ADC_OVSR_X8, 8·é¤æ·é¤æ;
                                     //               ADC_OVSR_X16, 16·é¤æ·é¤æ; ADC_OVSR_X32, 32·é¤æ·é¤æ; ADC_OVSR_X64, 64·é¤æ·é¤æ; ADC_OVSR_X128, 128·é¤æ·é¤æ;
                                     //               ADC_OVSR_X256, 256·é¤æ·é¤æ; ADC_OVSR_X512, 512·é¤æ·é¤æ; ADC_OVSR_X1024, 1024·é¤æ·é¤æ;    

#define ADC_CLOCK      3             // ADC¶é¥è¹æ: 
                                     // 0(ADC_CLOCK_6_25MHZ): 6.25MHZ; 1(ADC_CLOCK_12_5MHZ):12.5MHZ: 
                                     // 2(ADC_CLOCK_25MHZ): 25MHZ;     3(ADC_CLOCK_50MHZ): 50MHZ

#if (ADC_MODE == ADC_MODE_SWSTART)
#define ADC_TIM14        ADC_TIM14MAIN_FLAG // è®¹æ¤æADC_TIM14MAIN_FLAG
#endif                                       

#if (ADC_MODE == ADC_MODE_EXTSEL)
#define ADC_EXTSEL     ADC_EXTSEL_T8TRGO   // ¤æAD·é¤æ·é¤ææº: ADC_EXTSEL_EXTI11/ADC_EXTSEL_T8TRGO/ADC_EXTSEL_LPTIM1_OUT/ADC_EXTSEL_LPTIM2_OUT/ADC_EXTSEL_LPTIM3_OUT
#endif                                      


// ·é¥ç¡æ·é¤æ·é¤æ
#define AI1_RANGE     1      // AI1·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI2_RANGE     1      // AI2·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI3_RANGE     1      // AI3·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI4_RANGE     1      // AI4·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI5_RANGE     1      // AI5·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI6_RANGE     1      // AI6·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI7_RANGE     1      // AI7·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA;
#define AI8_RANGE     1      // AI8·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI9_RANGE     0      // AI9·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 
#define AI10_RANGE    0      // AI10·é¤æ·é¤æ: 0, ¤æ·å(0~4095); 1, 0~+10V; 2, -10V~+10V; 3, 0~5V; 4, -5V~+5V; 5, 0~+20mA; 6, -20mA~+20mA; 

// DC·é¤æè®¹æMA·å¨é¤æ
#define ADC_DMA_EN    1      // ADC DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif

/************************************************************************************/
//  DAC1,DAC2·é¤æ
/************************************************************************************/
// DAC1
#define DAC1_EN            1		// DAC1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (DAC1_EN>0)

#define DAC1_MODE		   0		// DAC1æ¨¡å0(DAC_MODE_MTOUT),     ¹æ·é; 
                                    //           1(DAC_MODE_ATOUT_N),   ·é¤æ·é1~N·é¤æ·éç¢·é·ç·åæ­; 
                                    //           2(DAC_MODE_ATOUT),     ·é¤æ·é¤æ·é¤ææ¢°é¤æ·é, ·åæ­;
                                    //           3(DAC_MODE_NOISE),     ·é¤æ·é¤æ·æ¨¡å¼;
                                    //           4(DAC_MODE_TRIANGLE),  ·é¤æ·éè§§ææ¨¡å;
                                    
#define DAC1_TRIGGER_MODE  DAC_TRIGGER_NONE   	// DAC1·é¤æ·éä¾¥ï¼ª: 
                                                // DAC_TRIGGER_NONE,     è¾¾æ;
                                                // DAC_TRIGGER_SOFTWARE, ·é¤æ·é¤æ;
                                                // DAC_TRIGGER_TIM6,     TIM6·æ·é¤æ
                                                // DAC_TRIGGER_EXTI9,    EXTI9ç¡·æ·é¤æ
                                    
#define DAC1_FREQ		   1000   	// DAC1è®¹æ·é¡ç
#define DAC1_OUTBUF_EN	   1     	// DAC1·é¤æ·éç»·é: 1, ä½¿é¤æ; 0, ;
#define DAC1_SCAN_T        3000     // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#if ((DAC1_MODE == DAC_MODE_ATOUT_N)||(DAC1_MODE == DAC_MODE_ATOUT))
#define TIM6_DAC1_EN       1        // ·éè®¹æ¶é¤æ6·é¤æDAC1·é¤æ, ·é¤æ·è·ç·é
#define DAC1_TXBUF_SIZE    256 	    // DAC1·é¤æ·é·ä·é¥é

#define DAC1_DMA_EN        1        // DAC1 DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif

#endif

// DAC2
#define DAC2_EN            1		// DAC2ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#if (DAC2_EN>0)
#define DAC2_MODE		   0		// DAC2æ¨¡å0(DAC_MODE_MTOUT),     ¹æ·é; 
                                    //           1(DAC_MODE_ATOUT_N),   ·é¤æ·é1~N·é¤æ·éç¢·é·ç·åæ­; 
                                    //           2(DAC_MODE_ATOUT),     ·é¤æ·é¤æ·é¤ææ¢°é¤æ·é, ·åæ­;     
                                    //           3(DAC_MODE_NOISE),     ·é¤æ·é¤æ·æ¨¡å¼;
                                    //           4(DAC_MODE_TRIANGLE),  ·é¤æ·éè§§ææ¨¡å;
                                    
#define DAC2_TRIGGER_MODE  DAC_TRIGGER_NONE   	// DAC2·é¤æ·éä¾¥ï¼ª: 
                                                // DAC_TRIGGER_NONE,     è¾¾æ;
                                                // DAC_TRIGGER_SOFTWARE, ·é¤æ·é¤æ;
                                                // DAC_TRIGGER_TIM7,     TIM7·æ·é¤æ
                                                // DAC_TRIGGER_EXTI9,    EXTI9ç¡·æ·é¤æ
                                                
#define DAC2_FREQ		   1000   	// DAC2è®¹æ·é¡ç
#define DAC2_OUTBUF_EN	   1     	// DAC1·é¤æ·éç»·é: 1, ä½¿é¤æ; 0, ;
#define DAC2_SCAN_T        3000     // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#if ((DAC2_MODE == DAC_MODE_ATOUT_N)||(DAC1_MODE == DAC_MODE_ATOUT))
#define TIM7_DAC2_EN       1        // ·éè®¹æ¶é¤æ7·é¤æDAC2·é¤æ, ·é¤æ·è·ç·é
#define DAC2_TXBUF_SIZE    256 	    // DAC2·é¤æ·é·ä·é¥é

#define DAC2_DMA_EN        1        // DAC2 DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif

#endif
/************************************************************************************/
// PWM1 ·é¤æ·é(è®¹æ¶é¤æ1)
// JP8: PWM1(PUL1+/PUL1-),DIR1(DIR1+/DIR1-)NA1(ENA1-)
/************************************************************************************/
#define PWM1_EN             0		   // PWM1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (PWM1_EN > 0)

// ·é¤æPWM·éä¾¥ï¼ª¤æ·é
#define PWM1_MODE           PWM_FREQ  // ·é¤æ¤æ: 0(PWM_FREQ):   ·é¤æ·é¤æé¢¤æ·é, ·é¤æ·é
                                       //           1(PWM_FREQ_N): ·é¤æ·é¡ç·é¤æ, ·é¤æ·ç·é¤æ·é¤æ·åæ­
                                       //           2(PWM_RATE):   è®¹æé¢¤æç§¸æç¢·é¤æ·é¤æ·é: ·éçµM, é¢ç»è®¹æ,ç§¸æ0%-100%¼´ç¢, ·é¤æ·é
                                       //           3(PWM_WRITE):  ·é¤æ·é¤ææ¨¡å, ·é¤æPWM_Write()·é¤æå®¤æ·é¤æ·éä¾¥ï¼ª, ·é¤æ·å¤æ¤æ·é¤æ·é¤æ, 
                                       //                          ·éç»·é¤æ´»MA·é¤æMA·å·é¤æPWM, ·çº¦MCU·æ
#define PWM1_SCAN_T         100        // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define PWM1_FREQ		    1000  	   // ·å¤æ
#define PWM1_TIM            TIM1_ID   // ¤æ¶é¤æ, ·é¤æ·è·ç·é
#define TIM1_PWM_EN         1          // ·éè®¹æ¶é¤æ1·é¤æPWM·é¤æ, ·é¤æ·è·ç·é


// ¤æä½¿é¤æ
#define PWM1CH1_EN		    1		   // PWM1CH11, ä½¿é¤æ; 0, 
#define PWM1CH2_EN		    0		   // PWM1CH21, ä½¿é¤æ; 0, 
#define PWM1CH3_EN		    0		   // PWM1CH31, ä½¿é¤æ; 0, 
#define PWM1CH4_EN		    0		   // PWM1CH41, ä½¿é¤æ; 0, 

// PWM·é¤æ¤æä½¿é¤æ 
#define PWM1CH_EN		   (PWM1CH1_EN|(PWM1CH2_EN<<1)|(PWM1CH3_EN<<2)|(PWM1CH4_EN<<3))  // PWM1·é¤æ¤æä½¿é°ïIT0:CH1;BIT1:CH2;BIT2:CH3;BIT3:CH4;

#define PWM1CH1_RATE		500        // PWM1CH1·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM1CH2_RATE		500        // PWM1CH2·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM1CH3_RATE		500        // PWM1CH3·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM1CH4_RATE		500        // PWM1CH4·å§¸50%(0(0.0%)~1000(100.0%))

#define PWM1CH1_PIN		    0          // PWM1CH1¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM1CH2_PIN		    0          // PWM1CH2¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM1CH3_PIN		    0          // PWM1CH3¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM1CH4_PIN		    0          // PWM1CH4¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹

#if (PWM1_MODE == PWM_WRITE)
#define PWM1_DMA_EN         1          // PWM1 DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif

// ·é¤æ¤æä½¿é¤æ(IM1/8·é¤æ¤æ·é¤æ·é, IM15/16/17H1¤æ·é¤æ·é)
#if ((PWM1_TIM == TIM1_ID)||(PWM1_TIM == TIM8_ID)||(PWM1_TIM == TIM15_ID)||(PWM1_TIM == TIM16_ID)||(PWM1_TIM == TIM17_ID))
#define PWM1CH1N_EN		    0		   // PWM1CH1N1, ä½¿é¤æ; 0, 
#define PWM1CH2N_EN		    0		   // PWM1CH2N1, ä½¿é¤æ; 0, 
#define PWM1CH3N_EN		    0		   // PWM1CH3N1, ä½¿é¤æ; 0, 

#define PWM1CH1N_PIN		0          // PWM1CH1N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹
#define PWM1CH2N_PIN		0          // PWM1CH2N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹
#define PWM1CH3N_PIN		0          // PWM1CH3N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹

#define PWM1_DTG		    1000	   // ·é¤ææ¨¡å·é¤æ¶é¤æ·éï½·äs
#define PWM1_BKIN_EN        0          // ¹é¤æ·é¤æä½¿é¤æ: 0, ; 1, ä½¿é¤æ
#define PWM1_BKIN2_EN       0          // ¹é¤æ·é¤æ2ä½¿é¤æ: 0, ; 1, ä½¿é¤æ
#endif

#endif
/************************************************************************************/
// PWM2·é¤æ·é(è®¹æ¶é¤æ2)
// JP12: PWM2(PUL2+/PUL2-),DIR2(DIR2+/DIR2-)NA2(ENA2-)
/************************************************************************************/
// PWM2 ·é¤æ
#define PWM2_EN             0		   // PWM2ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (PWM2_EN > 0)

// ·é¤æPWM·éä¾¥ï¼ª¤æ·é
#define PWM2_MODE           PWM_FREQ  // ·é¤æ¤æ: 0(PWM_FREQ):   ·é¤æ·é¤æé¢¤æ·é, ·é¤æ·é
                                       //           1(PWM_FREQ_N): ·é¤æ·é¡ç·é¤æ, ·é¤æ·ç·é¤æ·é¤æ·åæ­
                                       //           2(PWM_RATE):   è®¹æé¢¤æç§¸æç¢·é¤æ·é¤æ·é: ·éçµM, é¢ç»è®¹æ,ç§¸æ0%-100%¼´ç¢, ·é¤æ·é
                                       //           3(PWM_WRITE):  ·é¤æ·é¤ææ¨¡å, ·é¤æPWM_Write()·é¤æå®¤æ·é¤æ·éä¾¥ï¼ª, ·é¤æ·å¤æ¤æ·é¤æ·é¤æ, 
                                       //                          ·éç»·é¤æ´»MA·é¤æMA·å·é¤æPWM, ·çº¦MCU·æ
#define PWM2_SCAN_T         100        // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

#define PWM2_FREQ		    1000	   // ·å¤æ
#define PWM2_TIM        	TIM2_ID    // ¤æ¶é¤æ, ·é¤æ·è·ç·é
#define TIM2_PWM_EN         1          // ·éè®¹æ¶é¤æ2·é¤æPWM·é¤æ, ·é¤æ·è·ç·é

#define PWM2CH1_EN		    1		   // PWM1CH11, ä½¿é¤æ; 0, 
#define PWM2CH2_EN		    0		   // PWM1CH21, ä½¿é¤æ; 0, 
#define PWM2CH3_EN		    0		   // PWM1CH31, ä½¿é¤æ; 0, 
#define PWM2CH4_EN		    0		   // PWM1CH41, ä½¿é¤æ; 0, 
// PWM·é¤æ¤æä½¿é¤æ 
#define PWM2CH_EN		   (PWM2CH1_EN|(PWM2CH2_EN<<1)|(PWM2CH3_EN<<2)|(PWM2CH4_EN<<3))  // PWM2·é¤æ¤æä½¿é°ïIT0:CH1;BIT1:CH2;BIT2:CH3;BIT3:CH4;

#define PWM2CH1_RATE		500        // PWM1CH1·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM2CH2_RATE		500        // PWM1CH2·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM2CH3_RATE		500        // PWM1CH3·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM2CH4_RATE		500        // PWM1CH4·å§¸50%(0(0.0%)~1000(100.0%))

#define PWM2CH1_PIN		    0          // PWM1CH1¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM2CH2_PIN		    0          // PWM1CH2¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM2CH3_PIN		    0          // PWM1CH3¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM2CH4_PIN		    0          // PWM1CH4¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹

#if (PWM2_MODE == PWM_WRITE)
#define PWM2_DMA_EN         1          // PWM2 DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif


// ·é¤æ¤æä½¿é¤æ(IM1/8·é¤æ¤æ·é¤æ·é, IM15/16/17H1¤æ·é¤æ·é)
#if ((PWM2_TIM == TIM1_ID)||(PWM2_TIM == TIM8_ID)||(PWM2_TIM == TIM15_ID)||(PWM2_TIM == TIM16_ID)||(PWM2_TIM == TIM17_ID))
#define PWM2CH1N_EN		    0		   // PWM2CH1N1, ä½¿é¤æ; 0, 
#define PWM2CH2N_EN		    0		   // PWM2CH2N1, ä½¿é¤æ; 0, 
#define PWM2CH3N_EN		    0		   // PWM2CH3N1, ä½¿é¤æ; 0, 

#define PWM2CH1N_PIN		0          // PWM2CH1N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹
#define PWM2CH2N_PIN		0          // PWM2CH2N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹
#define PWM2CH3N_PIN		0          // PWM2CH3N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹

#define PWM2_DTG		    1000	   // ·é¤ææ¨¡å·é¤æ¶é¤æ·éï½·äs
#define PWM2_BKIN_EN        0          // ¹é¤æ·é¤æä½¿é¤æ: 0, ; 1, ä½¿é¤æ
#define PWM2_BKIN2_EN       0          // ¹é¤æ·é¤æ2ä½¿é¤æ: 0, ; 1, ä½¿é¤æ
#endif


#endif
/************************************************************************************/
// PWM3·é¤æ·é(è®¹æ¶é¤æ3)    
// JP13: PWM3(PUL3+/PUL3-),DIR3(DIR3+/DIR3-)NA3(ENA3-)
/************************************************************************************/
// PWM3 ·é¤æ
#define PWM3_EN             0		   // PWM3ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (PWM3_EN > 0)

// ·é¤æPWM·éä¾¥ï¼ª¤æ·é
#define PWM3_MODE           PWM_FREQ   // ·é¤æ¤æ: 0(PWM_FREQ):   ·é¤æ·é¤æé¢¤æ·é, ·é¤æ·é
                                       //           1(PWM_FREQ_N): ·é¤æ·é¡ç·é¤æ, ·é¤æ·ç·é¤æ·é¤æ·åæ­
                                       //           2(PWM_RATE):   è®¹æé¢¤æç§¸æç¢·é¤æ·é¤æ·é: ·éçµM, é¢ç»è®¹æ,ç§¸æ0%-100%¼´ç¢, ·é¤æ·é
                                       //           3(PWM_WRITE):  ·é¤æ·é¤ææ¨¡å, ·é¤æPWM_Write()·é¤æå®¤æ·é¤æ·éä¾¥ï¼ª, ·é¤æ·å¤æ¤æ·é¤æ·é¤æ, 
                                       //                          ·éç»·é¤æ´»MA·é¤æMA·å·é¤æPWM, ·çº¦MCU·æ
#define PWM3_SCAN_T         100        // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
                                       
#define PWM3_FREQ		    1000	   // ·å¤æ
#define PWM3_TIM        	TIM3_ID    // ¤æ¶é¤æ, ·é¤æ·è·ç·é
#define TIM3_PWM_EN         1          // ·éè®¹æ¶é¤æ3·é¤æPWM·é¤æ, ·é¤æ·è·ç·é

#define PWM3CH1_EN		    1		   // PWM1CH11, ä½¿é¤æ; 0, 
#define PWM3CH2_EN		    0		   // PWM1CH21, ä½¿é¤æ; 0, 
#define PWM3CH3_EN		    0		   // PWM1CH31, ä½¿é¤æ; 0, 
#define PWM3CH4_EN		    0		   // PWM1CH41, ä½¿é¤æ; 0, 
// PWM·é¤æ¤æä½¿é¤æ 
#define PWM3CH_EN		   (PWM3CH1_EN|(PWM3CH2_EN<<1)|(PWM3CH3_EN<<2)|(PWM3CH4_EN<<3))  // PWM3·é¤æ¤æä½¿é°ïIT0:CH1;BIT1:CH2;BIT2:CH3;BIT3:CH4;


#define PWM3CH1_RATE		500        // PWM1CH1·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM3CH2_RATE		500        // PWM1CH2·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM3CH3_RATE		500        // PWM1CH3·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM3CH4_RATE		500        // PWM1CH4·å§¸50%(0(0.0%)~1000(100.0%))

#define PWM3CH1_PIN		    0          // PWM1CH1¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM3CH2_PIN		    0          // PWM1CH2¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM3CH3_PIN		    0          // PWM1CH3¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM3CH4_PIN		    0          // PWM1CH4¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹

#if (PWM3_MODE == PWM_WRITE)
#define PWM3_DMA_EN         1          // PWM3 DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif

#endif
/************************************************************************************/
// PWM4·é¤æ·é(è®¹æ¶é¤æ17) 
// JP4: PWM4(PUL4+/PUL4-),DIR4(DIR4+/DIR4-)NA4(ENA4-)
/************************************************************************************/
// PWM4 ·é¤æ
#define PWM4_EN             0		   // PWM4ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (PWM4_EN > 0)

// ·é¤æPWM·éä¾¥ï¼ª¤æ·é
#define PWM4_MODE           PWM_FREQ   // ·é¤æ¤æ: 0(PWM_FREQ):   ·é¤æ·é¤æé¢¤æ·é, ·é¤æ·é
                                       //           1(PWM_FREQ_N): ·é¤æ·é¡ç·é¤æ, ·é¤æ·ç·é¤æ·é¤æ·åæ­
                                       //           2(PWM_RATE):   è®¹æé¢¤æç§¸æç¢·é¤æ·é¤æ·é: ·éçµM, é¢ç»è®¹æ,ç§¸æ0%-100%¼´ç¢, ·é¤æ·é
                                       //           3(PWM_WRITE):  ·é¤æ·é¤ææ¨¡å, ·é¤æPWM_Write()·é¤æå®¤æ·é¤æ·éä¾¥ï¼ª, ·é¤æ·å¤æ¤æ·é¤æ·é¤æ, 
                                       //                          ·éç»·é¤æ´»MA·é¤æMA·å·é¤æPWM, ·çº¦MCU·æ
#define PWM4_SCAN_T         100        // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
                                       
#define PWM4_FREQ		    1000	   // ·å¤æ
#define PWM4_TIM        	TIM17_ID   // ¤æ¶é¤æ, ·é¤æ·è·ç·é
#define TIM17_PWM_EN        1          // ·éè®¹æ¶é¤æ17·é¤æPWM·é¤æ, ·é¤æ·è·ç·é

#define PWM4CH1_EN		    1		   // CH11, ä½¿é¤æ; 0, 
#define PWM4CH2_EN		    0		   // CH21, ä½¿é¤æ; 0, 
#define PWM4CH3_EN		    0		   // CH31, ä½¿é¤æ; 0, 
#define PWM4CH4_EN		    0		   // CH41, ä½¿é¤æ; 0, 
// PWM·é¤æ¤æä½¿é¤æ 
#define PWM4CH_EN		   (PWM4CH1_EN|(PWM4CH2_EN<<1)|(PWM4CH3_EN<<2)|(PWM4CH4_EN<<3))  // PWM4·é¤æ¤æä½¿é°ïIT0:CH1;BIT1:CH2;BIT2:CH3;BIT3:CH4;


#define PWM4CH1_RATE		500        // CH1·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM4CH2_RATE		500        // CH2·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM4CH3_RATE		500        // CH3·å§¸50%(0(0.0%)~1000(100.0%))
#define PWM4CH4_RATE		500        // CH4·å§¸50%(0(0.0%)~1000(100.0%))

#define PWM4CH1_PIN		    0          // CH1¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM4CH2_PIN		    0          // CH2¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM4CH3_PIN		    0          // CH3¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹
#define PWM4CH4_PIN		    0          // CH4¢æ¨¡å¼¤æ·èè¯ºé: 0, ç¢å¹; 1, ç¢å¹

#if (PWM4_MODE == PWM_WRITE)
#define PWM4_DMA_EN         1          // PWM4 DMAä½¿é¤æ, 1·é¤æä½¿é°ï 0·é; 
#endif


// ·é¤æ¤æä½¿é¤æ(IM1/8·é¤æ¤æ·é¤æ·é, IM15/16/17H1¤æ·é¤æ·é)
#if ((PWM3_TIM == TIM1_ID)||(PWM3_TIM == TIM8_ID)||(PWM3_TIM == TIM15_ID)||(PWM3_TIM == TIM16_ID)||(PWM3_TIM == TIM17_ID))
#define PWM3CH1N_EN		    0		   // PWM3CH1N1, ä½¿é¤æ; 0, 
#define PWM3CH2N_EN		    0		   // PWM3CH2N1, ä½¿é¤æ; 0, 
#define PWM3CH3N_EN		    0		   // PWM3CH3N1, ä½¿é¤æ; 0, 

#define PWM3CH1N_PIN		0          // PWM3CH1N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹
#define PWM3CH2N_PIN		0          // PWM3CH2N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹
#define PWM3CH3N_PIN		0          // PWM3CH3N¢æ¨¡å¼°æ·é¤æ·é(·æ·é¤æ·å¹³): 0, ç¢å¹; 1, ç¢å¹

#define PWM3_DTG		    1000	   // ·é¤ææ¨¡å·é¤æ¶é¤æ·éï½·äs
#define PWM3_BKIN_EN        0          // ¹é¤æ·é¤æä½¿é¤æ: 0, ; 1, ä½¿é¤æ
#define PWM3_BKIN2_EN       0          // ¹é¤æ·é¤æ2ä½¿é¤æ: 0, ; 1, ä½¿é¤æ
#endif

#endif

/************************************************************************************/
//  ·é¤æPWMä½¿é¤æ·ä½£·é
/************************************************************************************/
#define PWM_EN  (PWM1_EN+PWM2_EN+PWM3_EN+PWM4_EN)

/************************************************************************************/
// FCLK ·é¤æ·é¤æ·é¤æ(è®¹æ¶é¤æ24)
// JP14: 1, +VT; 2,3(FA+,FA-: FCLK1_CH1),PF11; 4,5(FB+,FB-: FCLK1_CH2), PF12; 6,7(FZ+,FZ-: FCLK1_CH3), PF13; 8, GND;
/************************************************************************************/
#define FCLK1_EN           1	       // FCLK1ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#if (FCLK1_EN > 0)
#define FCLK1_MODE	       2		   // æ¨¡å¤æ: 0(FCLK_MODE_COUNT), ·é¤ææ¨¡å(1è·, CH1·é¤æ·æ); 
                                       //           1(FCLK_MODE_DECODE), ·é¤æ·é¤æ·é¤æ(CH1H2);
                                       //           2(FCLK_MODE_FREQ), ·é¨¡å¼(4è·, CH1, CH2, CH3, CH4·éè¯½é¤æ); 
                                       //           3(FCLK_MODE_PWMRATE), WMç§¸æ·æ¨¡å¼(1è·, CH1·é¤æ·æ); 

#define FCLK1_SCAN_T       1000        // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#define FCLK1_ATOUT_T      3000        // ·é¤æè®¹æ·é¤æ, ·ä: ms

#if ((FCLK1_MODE == 2)||(FCLK1_MODE == 3))
#define FCLK1_IRQREAD_EN   0           // è®¹æ¨¡å¼½¿: 0, ·é¤æFCLK_Read()·é¤æ·å; 1, ·é¤æ´æCLK_IRQHandler()·é¤æ
                                       // æ³¨éè§£ï·é§æé¢¨¡å¼µè§æPWMç§¸æ·æ¨¡å¼¤æ
#endif

#define FCLK1_TIM          TIM8_ID     // ¤æ¶é¤æ, ·é¤æ·è·ç·é
#define TIM8_FCLK_EN       1           // ·éè®¹æ¶é¤æ24·é¤æFCLK·é¤æ, ·é¤æ·è·ç·é

// ç¡·æ·æ¨¡å¼µè§æPWMç§¸æ·æ¨¡å¼¤æFCLK1CH1_EN·é¤æä½¿é¤æ
// ·é¤æ·é¤æ·é¤æ·é¤ææ¨¡åCLK1CH1_EN, FCLK1CH2_EN·é¤æä½¿é¤æ
// è¯§æé¢¨¡å¼¤æ·å·ç¡¬·é¤æ¸æ¨é¤æ 
#define FCLK1CH1_EN		   1	       // FCLK1CH11, ä½¿é¤æ; 0, 
#define FCLK1CH2_EN		   1		   // FCLK1CH21, ä½¿é¤æ; 0, 
#define FCLK1CH3_EN		   1	       // FCLK1CH31, ä½¿é¤æ; 0, 
#define FCLK1CH4_EN		   0		   // FCLK1CH41, ä½¿é¤æ; 0, 


// FCLK1·é¤æ¤æä½¿é¤æ 
#define FCLK1CH_EN		   (FCLK1CH1_EN|(FCLK1CH2_EN<<1)|(FCLK1CH3_EN<<2)|(FCLK1CH4_EN<<3))  // FCLK1·é¤æ¤æä½¿é°ïIT0:CH1;BIT1:CH2;BIT2:CH3;BIT3:CH4;

#define FCLK1_MINFREQ	   100     	   // æ¨¡å23ï½·é¤æ·åé¢¤æ, ·ähz
 
#define FCLK1CH1_PIN	   0           // FCLK1¤æ·é¤æ·è¾¾·é¤æ·éï½ 0, ·é¤æ; 1, ¤æ
#define FCLK1CH2_PIN	   0           // FCLK2¤æ·é¤æ·è¾¾·é¤æ·éï½ 0, ·é¤æ; 1, ¤æ
#define FCLK1CH3_PIN	   0           // FCLK3¤æ·é¤æ·è¾¾·é¤æ·éï½ 0, ·é¤æ; 1, ¤æ
#define FCLK1CH4_PIN	   0           // FCLK4¤æ·é¤æ·è¾¾·é¤æ·éï½ 0, ·é¤æ; 1, ¤æ

#define FCLK1CH1_PCS	   0           // CH1¤æ·é¤æ´æé¢¤æé¢³»·é¤æ 0, ·é¤æé¢; 1, 2·é; 2, 4·é; 3, 8·é;
#define FCLK1CH2_PCS	   0           // CH2¤æ·é¤æ´æé¢¤æé¢³»·é¤æ 0, ·é¤æé¢; 1, 2·é; 2, 4·é; 3, 8·é;
#define FCLK1CH3_PCS	   0           // CH3¤æ·é¤æ´æé¢¤æé¢³»·é¤æ 0, ·é¤æé¢; 1, 2·é; 2, 4·é; 3, 8·é;
#define FCLK1CH4_PCS	   0           // CH4¤æ·é¤æ´æé¢¤æé¢³»·é¤æ 0, ·é¤æé¢; 1, 2·é; 2, 4·é; 3, 8·é;


#if ((FCLK1_MODE == 2)||(FCLK1_MODE == 3))
#if (FCLK1CH1_EN > 0)
#define FCLK1CH1_BUF_SIZE  16         // FCLK1CH1·é¥é,·å 1~64
#endif
#if (FCLK1CH2_EN > 0)
#define FCLK1CH2_BUF_SIZE  16         // FCLK1CH2·é¥é,·å 1~64
#endif
#if (FCLK1CH3_EN > 0)
#define FCLK1CH3_BUF_SIZE  16         // FCLK1CH3·é¥é,·å 1~64
#endif
#if (FCLK1CH4_EN > 0)
#define FCLK1CH4_BUF_SIZE  16         // FCLK1CH4·é¥é,·å 1~64
#endif
#endif

// FCLK1·é¤æ¤æDMAä½¿é¤æ, è¯§æé¢¨¡å¼µè§æPWMç§¸æ·æ¨¡å¼¤æMA·é¤æ
// æ³¨é¤æ·é¤æFCLK·é¤æ·é¤æä¸·é¤æDMAä½¿é¤æ
#if (FCLK1_MODE > 1)
#define FCLK1CH1_DMA_EN		0	         // CH1 DMA1, ä½¿é¤æ; 0, ;  
#define FCLK1CH2_DMA_EN		0	         // CH2 DMA1, ä½¿é¤æ; 0, ;  
#define FCLK1CH3_DMA_EN	    0	         // CH3 DMA1, ä½¿é¤æ; 0, ;  
#define FCLK1CH4_DMA_EN	    0	         // CH3 DMA1, ä½¿é¤æ; 0, ;  

#endif

#endif
/************************************************************************************/
//  ·é¤æFCLKä½¿é¤æ·ä½£·é
/************************************************************************************/
#define FCLK_EN  (FCLK1_EN)

/************************************************************************************/
//  ·æ1·é¤æ 
/************************************************************************************/
#define TIM1_EN       0

#define TIM1_MODE     0		  // TIM1·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM1_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é
#define TIM1_T        1000000	  // TIM1·æ·é¤æ¶æ, ·äus
#define TIM1_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
                                        
#if ((TIM1_PWM_EN + TIM1_FCLK_EN + TIM1_EN)>1)
  #error "ERROR: TIM1 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM1 µÄÆäÖÐÒ»Ïî!"
#endif                                        
/************************************************************************************/
//  ·æ2·é¤æ 
/************************************************************************************/
#define TIM2_EN       0		  // TIM2ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM2_MODE     0		  // TIM2·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM2_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é
#define TIM2_T        1000000	  // TIM2·æ·é¤æ¶æ, ·äus					  
#define TIM2_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM2_PWM_EN + TIM2_FCLK_EN + TIM2_EN)>1)
  #error "ERROR: TIM2 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM2 µÄÆäÖÐÒ»Ïî!"
#endif
/************************************************************************************/
//  ·æ3·é¤æ 
/************************************************************************************/
#define TIM3_EN       0		  // TIM3ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM3_MODE     0		  // TIM3·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM3_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ

// ·å¤æ¶æ·é
#define TIM3_T        1000000	  // TIM3·æ·é¤æ¶æ, ·äus
#define TIM3_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM3_PWM_EN + TIM3_FCLK_EN + TIM3_EN)>1)
  #error "ERROR: TIM3 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM3 µÄÆäÖÐÒ»Ïî!"
#endif
/************************************************************************************/
//  ·æ4·é¤æ 
/************************************************************************************/
#define TIM4_EN       0		  // TIM4ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM4_MODE     0		  // TIM4·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM4_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é
#define TIM4_T        1000000	  // TIM4·æ·é¤æ¶æ, ·äus		
#define TIM4_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM4_PWM_EN + TIM4_FCLK_EN + TIM4_EN)>1)
  #error "ERROR: TIM4 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM4 µÄÆäÖÐÒ»Ïî!"
#endif
/************************************************************************************/
//  ·æ5·é¤æ 
/************************************************************************************/
#define TIM5_EN       0		  // TIM5ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM5_MODE     0		  // TIM5·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM5_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é
#define TIM5_T        1000000	  // TIM5·æ·é¤æ¶æ, ·äus
#define TIM5_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM5_PWM_EN + TIM5_FCLK_EN + TIM5_EN)>1)
  #error "ERROR: TIM5 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM5 µÄÆäÖÐÒ»Ïî!"
#endif
                                  
/************************************************************************************/
//  ·æ6·é¤æ 
/************************************************************************************/
#define TIM6_EN       0		  // TIM6ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM6_MODE     0		  // TIM6·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM6_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é 
#define TIM6_T        1000000	  // TIM6·æ·é¤æ¶æ, ·äus
#define TIM6_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM6_DAC1_EN>0)&&(TIM6_EN>0))
  #error "ERROR: DAC1(µ¥Í¨µÀÎÞ»º³å)ÐèÒª TIM6 ´¥·¢, ÇëÊ¹ÄÜ TIM6!"
#endif

/************************************************************************************/
//  ·æ7·é¤æ 
/************************************************************************************/
#define TIM7_EN       0	  // TIM7ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM7_MODE     0	  // TIM7·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM7_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é 
#define TIM7_T        1000000	  // TIM7·æ·é¤æ¶æ, ·äus

#define TIM7_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us

#if ((TIM7_DAC2_EN>0)&&(TIM7_EN>0))
  #error "ERROR: DAC2(µ¥Í¨µÀÎÞ»º³å)ÐèÒª TIM7 ´¥·¢, ÇëÊ¹ÄÜ TIM7!"
#endif                              
/************************************************************************************/
//  ·æ8·é¤æ 
//  æ³¨é¤æ: ·éç´C_ENä½¿é°è§æ·æ¨¡å¼¤æADC_MODE_EXTSEL, ·éè¯å°IM8_EN·é¤æä¸0
/************************************************************************************/
#if ((ADC_EN > 0)&&(ADC_MODE == ADC_MODE_EXTSEL)&&(ADC_EXTSEL == ADC_EXTSEL_T8TRGO))  // TIM8 ´¥·¢ ADC ×ª»»
#define TIM8_EN       0		  // ·é¤æ·é¤æä¸0
#else
#define TIM8_EN       0		  // TIM8ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#endif

#define TIM8_MODE     0		  // TIM8·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM8_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ


// ·å¤æ¶æ·é 
#define TIM8_T        1000000	  // TIM8·æ·é¤æ¶æ, ·äus
					  
#define TIM8_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM8_PWM_EN + TIM8_FCLK_EN + TIM8_EN)>1)
  #error "ERROR: TIM8 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM8 µÄÆäÖÐÒ»Ïî!"
#endif

/************************************************************************************/
//  ·æ12·é¤æ: ·é¤æ¶é¤æè®¹æ·é¤æ, ·é¤æä¼·è©æ, ·å½±Vars->Timerè¯§æ·é¤æ¡®
/************************************************************************************/
#define TIM12_EN       1		  // TIM12ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM12_MODE     1		  // TIM12·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM12_T·é¤æ; 
                                  //                1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                                  // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ

// ·å¤æ¶æ·é 
#define TIM12_T        1000000	  // TIM12·æ·é¤æ¶æ, ·äus

#define TIM12_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                         // é»è¾¡··é¤æ·ä1us
#if ((TIM12_PWM_EN + TIM12_FCLK_EN + TIM12_EN)>1)
  #error "ERROR: TIM12 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM12 µÄÆäÖÐÒ»Ïî!"
#endif

#if ((TIM12_EN != 1)||(TIM12_MODE != 1))
  #error "ERROR: TIM12 ²»ÄÜÍ¬Ê±Ê¹ÄÜ¶àÖÖÄ£Ê½, Çë¼ì²é PWM/FCLK/ENCODER Ä£Ê½ÅäÖÃ!"
#endif

/************************************************************************************/
//  ·æ13·é¤æ
/************************************************************************************/
#define TIM13_EN       1		  // TIM13ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM13_MODE     0		  // TIM13·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM13_T·é¤æ; 
                                  //                1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                                  // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ

// ·å¤æ¶æ·é
#define TIM13_T        1000000    // TIM13·æ·é¤æ¶æ, ·äus

#define TIM13_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                         // é»è¾¡··é¤æ·ä1us                                         
#if ((TIM13_PWM_EN + TIM13_FCLK_EN + TIM13_EN)>1)
  #error "ERROR: TIM13 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM13 µÄÆäÖÐÒ»Ïî!"
#endif

#if ((TIM13_EN != 1)||(TIM13_MODE != 0))
  #error "ERROR: TIM12 ²»ÄÜÍ¬Ê±Ê¹ÄÜ¶àÖÖÄ£Ê½, Çë¼ì²é PWM/FCLK/ENCODER Ä£Ê½ÅäÖÃ!"
#endif
/************************************************************************************/
//  ·æ14·é¤æ 
//  æ³¨é¤æ: ·éç´C_ENä½¿é°è§æ·æ¨¡å¼¤æADC_MODE_SWSTART, ·éè¯å°IM14_EN·é¤æä¸0
/************************************************************************************/
#if ((ADC_EN > 0)&&(ADC_MODE == ADC_MODE_SWSTART)) // TIM14 ´¥·¢ ADC ×ª»»
#define TIM14_EN       0		  // ·é¤æ·é¤æä¸0
#else
#define TIM14_EN       0		  // TIM14ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#endif

#define TIM14_MODE     0		  // TIM14·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM14_T·é¤æ; 
                                  //                1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                                  // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ

// ·å¤æ¶æ·é 
#define TIM14_T        1000000	  // TIM14·æ·é¤æ¶æ, ·äus

#define TIM14_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                         // é»è¾¡··é¤æ·ä1us
#if ((TIM14_PWM_EN + TIM14_FCLK_EN + TIM14_EN)>1)
  #error "ERROR: TIM14 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM14 µÄÆäÖÐÒ»Ïî!"
#endif  


/************************************************************************************/
//  ·æ15·é¤æ 
/************************************************************************************/
#define TIM15_EN       0		  // TIM15ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM15_MODE     0		  // TIM15·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM9_T·é¤æ; 
                              //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                              // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ
// ·å¤æ¶æ·é 
#define TIM15_T        1000000	  // TIM15·æ·é¤æ¶æ, ·äus

#define TIM15_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                        // é»è¾¡··é¤æ·ä1us
#if ((TIM15_PWM_EN + TIM15_FCLK_EN + TIM15_EN)>1)
  #error "ERROR: TIM15 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM15 µÄÆäÖÐÒ»Ïî!"
#endif
/************************************************************************************/
//  ·æ16·é¤æ 
/************************************************************************************/
#define TIM16_EN       0		  // TIM16ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM16_MODE     0		  // TIM16·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM10_T·é¤æ; 
                                  //                1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                                  // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ

// ·å¤æ¶æ·é¤æ
#define TIM16_T        1000000	  // TIM16·æ·é¤æ¶æ, ·äus

#define TIM16_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                         // é»è¾¡··é¤æ·ä1us
#if ((TIM16_PWM_EN + TIM16_FCLK_EN + TIM16_EN)>1)
  #error "ERROR: TIM16 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM16 µÄÆäÖÐÒ»Ïî!"
#endif

/************************************************************************************/
//  ·æ17·é¤æ 
/************************************************************************************/
#define TIM17_EN       0		  // TIM17ä½¿é¤æ, 1·é¤æä½¿é°ï 0·é

#define TIM17_MODE     0		  // TIM17·é¤ææ¨¡å: 0, TIM_WKMODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM11_T·é¤æ; 
                                  //               1, TIM_WKMODE_COUNT, ·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æTimer_Ctrl·é¤æ·é¤æCMD_TIM_ENA/CMD_TIM_DIS·é¤æ/¢é¤æ¶é¤æ,
                                  // ·é¤æ·é¤æCMD_TIM_READ·å·é¤æ

// ·å¤æ¶æ·é
#define TIM17_T        1000000	  // TIM17·æ·é¤æ¶æ, ·äus

#define TIM17_PSC      (SYSCLK/1000000)  // ·é³», ·é¤æ·æ¨¡å¼¤æ·ä¸ºTIM_WKMODE_COUNT, ·é¤æ·é¤æ;
                                         // é»è¾¡··é¤æ·ä1us
#if ((TIM17_PWM_EN + TIM17_FCLK_EN + TIM17_EN)>1)
  #error "ERROR: TIM17 ²»ÄÜÍ¬Ê±Ê¹ÄÜ PWM ºÍ FCLK, Çë¹Ø±Õ TIM17 µÄÆäÖÐÒ»Ïî!"
#endif

/************************************************************************************/
//  ·éè®¹æ¶é¤æä½¿é¤æ·ä½£·é
/************************************************************************************/
// TIMX Ê¹ÄÜ: 0=½ûÓÃ, 1=ÆôÓÃ
#define TIMX_EN  (TIM1_EN+TIM2_EN+TIM3_EN+TIM4_EN+TIM5_EN+TIM6_EN+TIM7_EN+TIM8_EN \
                     +TIM12_EN+TIM13_EN+TIM14_EN+TIM15_EN+TIM16_EN+TIM17_EN)



/************************************************************************************/
// ·æPTIM1·é¤æ
/************************************************************************************/
#define LPTIM1_EN       0

#define LPTIM1_MODE     0         // LPTIM·é¤ææ¨¡å:  0, LPTIM_MODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM1_T·é¤æ; 
                                  //                 1, LPTIM_MODE_COUNT,·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æLPTimer_Ctrl·é¤æ·é¤æCMD_LPTIM_ENA·é¤æ·é¤æ¶é¤æ, ·é¤æ·é¤æCMD_LPTIM_READ·å·é¤æ, ¶é¤æ¤æ·é¤æCMD_LPTIM_DIS¢é¤æ¶é¤æ
                                  //                 2, LPTIM_MODE_PWM, PWM·éä¾¥ï¼ª(OUT·é);
                                  //                 3, LPTIM_MODE_FCLK_COUNT, ·é¤æ·é¤æ·é(IN1·é¤æ);  
                                  //                 4, LPTIM_MODE_FCLK_DECOUNT, ·é¤æ·é¤æ·é¤æ·é¤æ·é¤æ(IN1,IN2·é¤æ); 
                                  //                 5, LPTIM_MODE_OUT, ·æ·é¤æ·é¤æ¾¥ï¼ 
// ·å¤æ¶æ·é
#define LPTIM1_CLK      LPTIM_LSE_32768HZ	      // ·æ·æ·æ¤æ
#define LPTIM1_T        1000000	                  // ·æ·é¤æ¶æ, ·äus
#define LPTIM1_PSC      LPTIM_PRESCALER_DIV1      // ·é³»: LPTIM_PRESCALER_DIV1/LPTIM_PRESCALER_DIV2/LPTIM_PRESCALER_DIV4/LPTIM_PRESCALER_DIV8
                                                  //           LPTIM_PRESCALER_DIV16/LPTIM_PRESCALER_DIV32/LPTIM_PRESCALER_DIV64/LPTIM_PRESCALER_DIV128

/************************************************************************************/
// ·æPTIM2·é¤æ
/************************************************************************************/
#if ((ADC_EN > 0)&&(ADC_MODE == ADC_MODE_EXTSEL)&&(ADC_EXTSEL == ADC_EXTSEL_LPTIM2_OUT))  // LPTIM2 ´¥·¢ ADC ×ª»»
#define LPTIM2_EN       0
#else
#define LPTIM2_EN       0
#endif

#define LPTIM2_MODE     0         // LPTIM·é¤ææ¨¡å:  0, LPTIM_MODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM1_T·é¤æ; 
                                  //                 1, LPTIM_MODE_COUNT,·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æLPTimer_Ctrl·é¤æ·é¤æCMD_LPTIM_ENA·é¤æ·é¤æ¶é¤æ, ·é¤æ·é¤æCMD_LPTIM_READ·å·é¤æ, ¶é¤æ¤æ·é¤æCMD_LPTIM_DIS¢é¤æ¶é¤æ
                                  //                 2, LPTIM_MODE_PWM, PWM·éä¾¥ï¼ª(OUT·é);
                                  //                 3, LPTIM_MODE_FCLK_COUNT, ·é¤æ·é¤æ·é(IN1·é¤æ);  
                                  //                 4, LPTIM_MODE_FCLK_DECOUNT, ·é¤æ·é¤æ·é¤æ·é¤æ·é¤æ(IN1,IN2·é¤æ); 
                                  //                 5, LPTIM_MODE_OUT, ·æ·é¤æ·é¤æ¾¥ï¼ 
// ·å¤æ¶æ·é
#define LPTIM2_CLK      LPTIM_LSE_32768HZ	      // ·æ·æ·æ¤æ
#define LPTIM2_T        1000000	                  // ·æ·é¤æ¶æ, ·äus
#define LPTIM2_PSC      LPTIM_PRESCALER_DIV1      // ·é³»: LPTIM_PRESCALER_DIV1/LPTIM_PRESCALER_DIV2/LPTIM_PRESCALER_DIV4/LPTIM_PRESCALER_DIV8
                                                  //           LPTIM_PRESCALER_DIV16/LPTIM_PRESCALER_DIV32/LPTIM_PRESCALER_DIV64/LPTIM_PRESCALER_DIV128
                                            
/************************************************************************************/
// ·æPTIM3·é¤æ
/************************************************************************************/
#define LPTIM3_EN       0

#define LPTIM3_MODE     0         // LPTIM·é¤ææ¨¡å:  0, LPTIM_MODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM1_T·é¤æ; 
                                  //                 1, LPTIM_MODE_COUNT,·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æLPTimer_Ctrl·é¤æ·é¤æCMD_LPTIM_ENA·é¤æ·é¤æ¶é¤æ, ·é¤æ·é¤æCMD_LPTIM_READ·å·é¤æ, ¶é¤æ¤æ·é¤æCMD_LPTIM_DIS¢é¤æ¶é¤æ
                                  //                 2, LPTIM_MODE_PWM, PWM·éä¾¥ï¼ª(OUT·é);
                                  //                 3, LPTIM_MODE_FCLK_COUNT, ·é¤æ·é¤æ·é(IN1·é¤æ);  
                                  //                 5, LPTIM_MODE_OUT, ·æ·é¤æ·é¤æ¾¥ï¼ 
// ·å¤æ¶æ·é
#define LPTIM3_CLK      LPTIM_LSE_32768HZ	      // ·æ·æ·æ¤æ
#define LPTIM3_T        1000000	                  // ·æ·é¤æ¶æ, ·äus
#define LPTIM3_PSC      LPTIM_PRESCALER_DIV1      // ·é³»: LPTIM_PRESCALER_DIV1/LPTIM_PRESCALER_DIV2/LPTIM_PRESCALER_DIV4/LPTIM_PRESCALER_DIV8
                                                  //           LPTIM_PRESCALER_DIV16/LPTIM_PRESCALER_DIV32/LPTIM_PRESCALER_DIV64/LPTIM_PRESCALER_DIV128
                                                  
/************************************************************************************/
// ·æPTIM4·é¤æ
/************************************************************************************/
#define LPTIM4_EN       0

#define LPTIM4_MODE     0         // LPTIM·é¤ææ¨¡å:  0, LPTIM_MODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM1_T·é¤æ; 
                                  //                 1, LPTIM_MODE_COUNT,·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æLPTimer_Ctrl·é¤æ·é¤æCMD_LPTIM_ENA·é¤æ·é¤æ¶é¤æ, ·é¤æ·é¤æCMD_LPTIM_READ·å·é¤æ, ¶é¤æ¤æ·é¤æCMD_LPTIM_DIS¢é¤æ¶é¤æ
                                  //                 2, LPTIM_MODE_PWM, PWM·éä¾¥ï¼ª(OUT·é);
                                  //                 5, LPTIM_MODE_OUT, ·æ·é¤æ·é¤æ¾¥ï¼ 
// ·å¤æ¶æ·é
#define LPTIM4_CLK      LPTIM_LSE_32768HZ	      // ·æ·æ·æ¤æ
#define LPTIM4_T        1000000	                  // ·æ·é¤æ¶æ, ·äus
#define LPTIM4_PSC      LPTIM_PRESCALER_DIV1      // ·é³»: LPTIM_PRESCALER_DIV1/LPTIM_PRESCALER_DIV2/LPTIM_PRESCALER_DIV4/LPTIM_PRESCALER_DIV8
                                                  //           LPTIM_PRESCALER_DIV16/LPTIM_PRESCALER_DIV32/LPTIM_PRESCALER_DIV64/LPTIM_PRESCALER_DIV128

/************************************************************************************/
// ·æPTIM5·é¤æ
/************************************************************************************/
#define LPTIM5_EN       1

#define LPTIM5_MODE     0         // LPTIM·é¤ææ¨¡å:  0, LPTIM_MODE_INT, ·æ·é¤æ·é¹æ¶éè®¹ææ¨¡å, ·æ¶é¤æ¼´è¯§æIM1_T·é¤æ; 
                                  //                 1, LPTIM_MODE_COUNT,·æ·é¤æ·é¹æ¶é¤æ·æ¨¡å¼, §£¨é¤æ·é¤æLPTimer_Ctrl·é¤æ·é¤æCMD_LPTIM_ENA·é¤æ·é¤æ¶é¤æ, ·é¤æ·é¤æCMD_LPTIM_READ·å·é¤æ, ¶é¤æ¤æ·é¤æCMD_LPTIM_DIS¢é¤æ¶é¤æ
                                  //                 2, LPTIM_MODE_PWM, PWM·éä¾¥ï¼ª(OUT·é);
                                  //                 5, LPTIM_MODE_OUT, ·æ·é¤æ·é¤æ¾¥ï¼ 
// ·å¤æ¶æ·é
#define LPTIM5_CLK      LPTIM_LSE_32768HZ	      // ·æ·æ·æ¤æ
#define LPTIM5_T        1000	                  // ·æ·é¤æ¶æ, ·äus
#define LPTIM5_PSC      LPTIM_PRESCALER_DIV1      // ·é³»: LPTIM_PRESCALER_DIV1/LPTIM_PRESCALER_DIV2/LPTIM_PRESCALER_DIV4/LPTIM_PRESCALER_DIV8
                                                  //           LPTIM_PRESCALER_DIV16/LPTIM_PRESCALER_DIV32/LPTIM_PRESCALER_DIV64/LPTIM_PRESCALER_DIV128

/************************************************************************************/
//  ·é¤æLPTIM·æ·ä½¿·é¤æä½£é¤æ
/************************************************************************************/
#define LPTIMX_EN  (LPTIM1_EN+LPTIM2_EN+LPTIM3_EN+LPTIM4_EN+LPTIM5_EN)

/************************************************************************************/
// SD·é¤æ: ·é¤æå¸D_MODE·é¤æ¤æ1, SDMMC·é¤ææ¨¡å
/************************************************************************************/
#define SDCARD_EN          1              // SD·ä½¿ï½1, ä½¿é¤æ;  0, ;

#if (SDCARD_EN > 0)
#define SD_MODE            SD_SDMMC_MODE  // SD·é¥å·æ¨¡å¼¤æ0(SD_SPI_MODE), SPI·é¤ææ¨¡å;  1(SD_SDMMC_MODE), SDMMC·é¤ææ¨¡å;
#define SD_PWR_EN          1              // SD·é¹æ·é¤æä½¿é°ï0, ¼´ç®¡è¡æ¸æ;  1, è®¹æ·é¤æ, ·é¤æ·é, ·é¤æç¢´é¥ç·é¤æ;

#if (SD_MODE == SD_SDMMC_MODE)
#define SD_SDMMC_ID  SDMMC1_ID
#define SD_BUS_WIDE  SDMMC_BUS_4BIT       // ¤æ1, SDMMC_BUS_1BIT; 4,SDMMC_BUS_4BIT;
#endif

#if (SD_MODE == SD_SPI_MODE)
#define SD_SPI_ID  SPI1_ID
#endif
#endif

/************************************************************************************/
// SPI FLASH(W25QXXç³»é¤æ)·é¤æ
/************************************************************************************/
#define SPIFLASH_EN            1        // SPI FLASHä½¿é°ï1, ä½¿é¤æ;  0, ;

#define SPIFLASH_MODE          1        // SPI FLASH·é¤æ·å1, ·é¤æFATFS¾¥ç¡·æç³»ç·éè®¹æ;  0, PI FLASH·å·é¤æ·éè¯§æ;
                                        // æ³¨é¤æ:2§æ·é¤æå¼·é¤æä¸    

#define SPIFLASH_TYPE          W25QXX   // SPI FLASH·é¤æ·é¤æ: ·é¤æ25QXX

#if (SPIFLASH_TYPE == W25QXX)
#define W25QXX_MODE            W25QXX_MODE_QSPI   // ·åæ¨¡å0(W25QXX_MODE_SPI), åº¤æSPI·é¤æ·é¤æ;  1(W25QXX_MODE_QSPI), åº¤æQSPI·é¤æ·é¤æ;
#if (W25QXX_MODE == W25QXX_MODE_SPI)
#define W25QXX_SPI_ID  SPI1_ID
#endif

// æ³¨é¤æ: ·ä¸ºW25QXX¸®·é¤æ·é¤æ, ·é·é¤æ¾¥ç¡·æç³»çå¸·é¤æ·é¤æ
#define W25QXX_MODEL           W25Q64    // SPI FLASH´æ

#define W25QXX_SECTOR_SIZE	   4096      // W25QXX·é¤æ·å 	

#define W25QXX_SECTOR_NUM	   2048      // SPI FLASH·é¤æ·é¤æ

#define W25QXX_FATFS_STARTSECTOR  0      // ·é·é¤æ¾¥ç¡·æç³»ç·é¤æå§¤æ, ·é¤æ0	
#define W25QXX_FATFS_SECTORNUM    1590   // ·é·é¤æ¾¥ç¡·æç³»ç·é¤æ·é¤æ, ·éä¼´ç©æ 	

#define W25QXX_SAVE_PARA_SECTOR   1599   // ·é¨ç³»ç»¤æ·é¤æ, ·éä¼´ç©æ 	

#define W25QXX_ZDY_STARTSECTOR    (W25QXX_SAVE_PARA_SECTOR+1)  // ·é¤æè®¹æ·é¤æ·é¤æå§¤æ 
#define W25QXX_ZDY_SECTORNUM      448	                       // ·é¤æè®¹æ·é¤æ·é¤æ

#define W25QXX_PAGE_SIZE	      256      // W25QXX·åé¡µé¤æå°, ·éç¼´é·é¤æ	

#define W25QXX_READ_DMA_PRIORITY_EN	  0    // W25QXX·é¤æDMA·å·ä½¿ï½1, ä½¿é¤æ;  0, ;
#define W25QXX_WRITE_PRIORITY_DMA_EN  0    // W25QXX·é¤æDMA·å½¿ï½1, ä½¿é¤æ;  0, ;

#endif

#if (SPIFLASH_EN == 0)
  #error "ERROR: SPIFLASH_EN ±ØÐëÏÈÊ¹ÄÜ!"
#endif


/************************************************************************************/
// USB·é¤ææ¨¡å·é¤æ
// æ³¨é¤æ: USB_HOST_ENSB_DEVICE_EN·é¤æä½¿é¤æ
/************************************************************************************/
#define USB_HOST_EN     1     // USB·é¤ææ¨¡åä½¿é°ï1, ä½¿é¤æ;  0, ;

#if (USB_HOST_EN > 0)

// U·ä½¿·é¤æ
#define UDISK_EN        1     // U·ä½¿ï½1, ä½¿é¤æ;  0, ;
#define USB_SCAN_T      10    // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#endif
/************************************************************************************/
// USB¨¡å¼¤æ
// æ³¨é¤æ: USB_HOST_ENSB_DEVICE_EN·é¤æä½¿é¤æ
// æ³¨éè§£ï·é¤æ·é¤æ·ææ²¡é¤æå®¤æ
/************************************************************************************/
#define USB_DEVICE_EN    0     // USB½¿ï½1, ä½¿é¤æ;  0, ;

#if (USB_DEVICE_EN > 0)
// USB, å®è¡·é¤æç¢SD·é¤æSPI FLASH·é¤æNAND FALSH·ä¸º¨é¤æ·åSB Mass Storage·é¤æ, 
// ·é¤æ·é¤æSB·çè®¹æD·é¤æSPI FLASH·é¤æNAND FLASH·éä¾¥ç¡·
// æ³: USB_VCP_ENSB_MSC_EN·é¤æä½¿é¤æ
#define USB_MSC_EN       1       //  USB Mass Storageä½¿é¤æ, 1·é¤æä½¿é°ï 0·é
#define USB_MSC_LUN      0       //  USB Mass Storage¨é¤æ·é¤æ: 0, SPI FLASH·é¤æä¸ºéç«ç¡·æ·é¤æ;
                                 //                                1, SD·é¤æ·ä¸ºç¡·æ·é¤æ;
						         //  ·é¤æ¼é¤æ
#define USB_SCAN_T       10    // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

// USB ·éè§£ä¸²·é¤æ·é¤æ
// æ³: USB_VCP_ENSB_MSC_EN·é¤æä½¿é¤æ
#define USB_VCP_EN       0        // USB VCP·éè§£ä¸²·ä½¿, 1·é¤æä½¿é°ï 0·é
#define USB_RXBUF_SIZE   512      // ·é¤æ·æ·å, ·é¤æ·é¤æ·é¡æ·ç·é¤æ·é¤æ·é·ç¹æ

#if ((USB_MSC_LUN == 0)&&(USB_MSC_EN > 0))
#if (SPIFLASH_MODE != 1)
  #error "ERROR: USB_MSC_EN Ê¹ÄÜÊ± USB_MSC_LUN ²»ÄÜÎª 0, Í¬Ê±ÐèÒª½« SPIFLASH_MODE ÖÃ 1"
#endif
#endif

#if ((USB_VCP_EN>0)&&(USB_MSC_EN>0))
  #error "ERROR: USB_VCP_EN ºÍ USB_MSC_EN ²»ÄÜÍ¬Ê±Ê¹ÄÜ!"
#endif

#endif

#if ((USB_DEVICE_EN>0)&&(USB_HOST_EN>0))
  #error "ERROR: USB_DEVICE_EN ºÍ USB_HOST_EN ²»ÄÜÍ¬Ê±Ê¹ÄÜ!"
#endif

/************************************************************************************/
// FatFS¾¥ç¡·æç³»ç·é¤æ
/************************************************************************************/
#define FATFS_EN               (SDCARD_EN|(SPIFLASH_EN&SPIFLASH_MODE)|UDISK_EN) // ¾¥ç¡·æç³»çä½¿é°ï1, ä½¿é¤æ;  0, ;
#define FATFS_SCAN_T           10    // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms

/************************************************************************************/
// TCPIP(LWIP)¤æ¤æ
/************************************************************************************/
#define LWIP_EN               TASK_LWIP_EN        // TCPIP(LWIP)¤æ½¿ï½1, ä½¿é¤æ;  0, ;

#if (OS_EN > 0)
#define LWIP_SCAN_T           20                  // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#else
#define LWIP_SCAN_T           1                   // ·éè®¹æ¶æ·æ·é¤æ, ·ä: ms
#endif

#define LWIP_CONFIG_EN        1                   // ·é¤æ¤æ: 1, ·é¤æ·é¤æ·é¤æ; 0, EPROM¨é¤æ·é¤æ·é¤æ

// ·é¤æ·é¤æ·é¤æ
//#define LOCAL_IP              "192.168.1.48"	  // ·é¤æIP
#define LOCAL_IP              "192.168.1.99"	  // ·é¤æIP
#define LOCAL_PORT      	  5000		   	      // ·éç«£ç
#define LOCAL_SUBNET_MASK     "255.255.255.0"     // ·é¤æ·é¤æ·é¤æ
#define LOCAL_GATEWAY         "192.168.1.1"       // ·é¤æ·é¤æ

/***************************************************************************************/
// ·é¤æ·ä¸ºTCP·é¤æ·æ¨¡å¼, ·é¤æ
#define LWIP_TCP_SERVER_EN      1     // TCP_SERVERä½¿é°ï1, ä½¿é¤æ;  0, ;  

#if (LWIP_TCP_SERVER_EN > 0)
#define MODBUS_TCP_EN           0     // Modbus TCPä½¿é¤æ: 1, §é¤æModbus TCP·é¤æä½¿é¤æ; 0, §é¤æ·é¤ææ¨¡å·é¤æ·é
                                      // æ³¨é¤æä½¿é¤æMODBUS_TCP_EN, ·ä½¿ODBUS_SLAVE_EN
                                      
#if (MODBUS_TCP_EN > 0)
#define TCP_SERVER_LOCAL_PORT   502   // ·éç«£ç
#else
#define TCP_SERVER_LOCAL_PORT   5000  // ·éç«£ç
#endif

// TCPä¼
#define LWIP_MAX_TCP_SERVER_LINK_NUM  4  // ·é¤æ·éçµPä¼©æ, ·é¤æ·é8
#endif

/***************************************************************************************/
// ·é¤æ·ä¸ºUDP·é¤æ·æ¨¡å¼, ·é¤æ
#define LWIP_UDP_SERVER_EN      0     // UDP_SERVERä½¿é°ï1, ä½¿é¤æ;  0, ;  
#if (LWIP_UDP_SERVER_EN > 0)
#define UDP_SERVER_LOCAL_PORT   5100  // ·éç«£ç
// UDPä¼
#define LWIP_MAX_UDP_SERVER_LINK_NUM    4  // ·é¤æ·éçµPä¼©æ, ·é8
#endif

/***************************************************************************************/
// ·é¤æ·ä¸ºTCPä¼·æ¨¡å¼, ·é¤æ
#define LWIP_TCP_CLIENT_EN      0     // TCP_CLIENTä½¿é°ï1, ä½¿é¤æ;  0, ;  
#if (LWIP_TCP_CLIENT_EN > 0)

#define TCP_CLIENT_LOCAL_PORT   5200  // ·éç«£ç

#define LWIP_TCP_DSC1_EN        1     // è¿¿å·é¤æ1ä½¿é°ï1, ä½¿é¤æ;  0, ; 
#define LWIP_TCP_DSC2_EN        0     // è¿¿å·é¤æ2ä½¿é°ï1, ä½¿é¤æ;  0, ; 
#define LWIP_TCP_DSC3_EN        0     // è¿¿å·é¤æ3ä½¿é°ï1, ä½¿é¤æ;  0, ; 
#define LWIP_TCP_DSC4_EN        0     // è¿¿å·é¤æ4ä½¿é°ï1, ä½¿é¤æ;  0, ; 

#define LWIP_MAX_TCP_DSC_NUM    2  	  // è¿¤æTCP·é¤æ·é¤æ, ·é4

#if (LWIP_TCP_DSC1_EN > 0)
#define LWIP_TCP_DSC1_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ1 IP
#define LWIP_TCP_DSC1_PORT         5201			      // è¿¿å·é¤æ1£ç
#endif

#if (LWIP_TCP_DSC2_EN > 0)
#define LWIP_TCP_DSC2_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ2 IP
#define LWIP_TCP_DSC2_PORT         5202			      // è¿¿å·é¤æ2£ç
#endif

#if (LWIP_TCP_DSC3_EN > 0)
#define LWIP_TCP_DSC3_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ3 IP
#define LWIP_TCP_DSC3_PORT         5203			      // è¿¿å·é¤æ3£ç
#endif

#if (LWIP_TCP_DSC4_EN > 0)
#define LWIP_TCP_DSC4_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ4 IP
#define LWIP_TCP_DSC4_PORT         5204			      // è¿¿å·é¤æ4£ç
#endif

#endif

/***************************************************************************************/
// ·é¤æ·ä¸ºUDPä¼·æ¨¡å¼, ·é¤æ
#define LWIP_UDP_CLIENT_EN      0     // UDP_CLIENTä½¿é°ï1, ä½¿é¤æ;  0, ;  
#if (LWIP_UDP_CLIENT_EN > 0)
#define UDP_CLIENT_LOCAL_PORT   5300  // ·éç«£ç

#define LWIP_MAX_UDP_DSC_NUM    2  	  // è¿¤æUDP·é¤æ·é¤æ·é¤æ·é4

#if (LWIP_MAX_UDP_DSC_NUM > 0)
#define LWIP_UDP_DSC1_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ1 IP
#define LWIP_UDP_DSC1_PORT         5301			      // è¿¿å·é¤æ1£ç
#endif
#if (LWIP_MAX_UDP_DSC_NUM > 1)
#define LWIP_UDP_DSC2_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ2 IP
#define LWIP_UDP_DSC2_PORT         5302			      // è¿¿å·é¤æ2£ç
#endif
#if (LWIP_MAX_UDP_DSC_NUM > 2)
#define LWIP_UDP_DSC3_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ3 IP
#define LWIP_UDP_DSC3_PORT         5303			      // è¿¿å·é¤æ3£ç
#endif
#if (LWIP_MAX_UDP_DSC_NUM > 3)
#define LWIP_UDP_DSC4_IP           "192.168.1.44"//"192.168.1.200"	  // è¿¿å·é¤æ4 IP
#define LWIP_UDP_DSC4_PORT         5304			      // è¿¿å·é¤æ4£ç
#endif
#endif

/***************************************************************************************/
// IAP ºä»¶º§½®
#define IAP_EN                    1     // IAP »å: 0=ç¦, 1=
#define IAP_YMODEM_EN             1     // UART YModem º§: 0=ç¦, 1= ( AT+FWU è§)
#define IAP_TFTP_EN               1     // TFTP ç½º§:   0=ç¦, 1=

/***************************************************************************************/
// TFTP ¨æ¨¡å¼ (è¦ IAP_EN && IAP_TFTP_EN )
#if ((IAP_EN > 0)&&(IAP_TFTP_EN>0))
#define LWIP_TFTP_SERVER_EN      1     // TFTP¨ä½¿: 1=, 0=ç¦
#else
#define LWIP_TFTP_SERVER_EN      0     // TFTP¨æ
#endif

#if (LWIP_TFTP_SERVER_EN > 0)       
#define TFTP_FILE_DISK           1   // TFTP·é¤æ¾¥ç¡·æ·é¤æ: 0, SPIFLASH_DISK, SPI FLASH·é¤æä¸ºéç«ç¡·æ·é¤æ0; 
                                     //                     1, SD_DISK, SD·é¤æ·ä¸ºç¡·æ·é¤æ1;
                                     //                     2, USB_DISK, U·é¤æ·ä¸ºç¡·æ·é¤æ2;
                                     //                     3, NFLASH_DISK, NAND FLASH·é¤æä¸ºéç«ç¡·æ·é¤æ3;

#define TFTP_RRQ_FILE_EN         0   // ·é¤æTFTP·å¾¥ç¡·æä½¿é¤æ: 1, ä½¿é¤æ;  0, ; 
#define TFTP_WRQ_FILE_EN         0   // ·é¤æTFTP¤æ¾¥ç¡·æä½¿é¤æ: 1, ä½¿é¤æ;  0, ; 

#define TFTP_WEQ_FILE_MODE       2   // 0, ·é¤æ¤æ·é, ¢å·é¤æè¾¾æ
                                     // 1, ·é¤æ¤æ·é, ·éä¾¥ç¡··é¤æ·é¤æ¾¥ç¡·æ·å°¾·é¤æ·é¤æ
                                     // 2, ·é¤æ¤æ·é, ¤æ¤æ¾¥ç¡·æ·é°æ·éä¾¥ç¡·
    

                                     
#define TFTP_SERVER_LOCAL_PORT   69  // ·éç«£ç
// ä¼
#define LWIP_MAX_TFTP_SERVER_LINK_NUM    2  // ·é¤æ·éçµTPä¼©æ, ·é4
#endif

/***************************************************************************************/
// ·é¤æ·ä¸ºHTTP·é¤æ·æ¨¡å¼, ·é¤æ
#define LWIP_HTTP_EN             0     // HTTPä½¿é°ï1, ä½¿é¤æ;  0, ; 

/***************************************************************************************/
// ·é¤æä¼·é¤æ
#define ETH_RXBUFNB           3
#define ETH_TXBUFNB           3
#define ETH_MAX_RX_PACKET_SIZE	  1520
#define ETH_MAX_TX_PACKET_SIZE	  1520

#if (MODBUS_TCP_EN > 0)
#if (MODBUS_SLAVE_EN!=1)
  #error "ERROR: MODBUS_TCP_EN Ê¹ÄÜÊ±±ØÐëÍ¬Ê±Ê¹ÄÜ MODBUS_SLAVE_EN!"
#endif
#endif

/************************************************************************************/
// SDRAM·é¤æ
/************************************************************************************/
#define SDRAM_EN          1               // 1, ä½¿é¤æ;  0, ;

#define SDRAM_LCD_START_ADDR  0xC1C00000  // LCD SDRAM·å¤æ(28MB·å·å)

/************************************************************************************/
// LTDC·é¤æ
/************************************************************************************/
#define LTDC_EN           0     // 1, ä½¿é¤æ;  0, ;
#if (LTDC_EN > 0)        

#define LCD_PRODUCT       LCD_ATK_MD0700R_800X480   // LCD·è¹æ: 0, LCD_ATK_MD0700R_800X480; 1,LCD_ATK_MD0700R_1024X600
//--------------------------------------------------------------------------------
// LCD·é¤æ·é¤æ·é¤æ
#define LCD_DISP_TYPE     0      // LCD·ç¤º·å: 0(LCD_TYPE_HS),·é¤æ; 1(LCD_TYPE_HS180),·é¤æ180; 
                                 //              2(LCD_TYPE_VS),·é¤æ; 3(LCD_TYPE_VS180),·é¤æ180; 

#if (LCD_PRODUCT == LCD_ATK_MD0700R_800X480)
#define LCD_WIDTH         800   // LCD·éç»
#define LCD_HEIGH         480   // LCD·éç»

#define LCD_FCLK          LTDC_CLK_32MHZ // LCD¶é¤æé¢¤æ: LTDC_CLK_32MHZ/LTDC_CLK_48MHZ/LTDC_CLK_64MHZ/LTDC_CLK_12MHZ
                                         // ä¸·ç·åç¢ä½¿éè¾: 800*480->LTDC_CLK_32MHZ; 1024*600->LTDC_CLK_48MHZ;
                                         //                           480*272->LTDC_CLK_12MHZ;  
#define LCD_HSYNC_WIDTH   20    // HSYNC·å·é¤æ
#define LCD_HBP_WIDTH     46    // HBP·å·é¤æ
#define LCD_HFP_WIDTH     210   // HFP·å·é¤æ

#define LCD_VSYNC_WIDTH   10    // VSYNC·å·é¤æ
#define LCD_VBP_WIDTH     23    // VBP·å·é¤æ
#define LCD_VFP_WIDTH     22    // VFP·å·é¤æ
#endif

#if (LCD_PRODUCT == LCD_ATK_MD0700R_1024X600)
#define LCD_WIDTH         1024   // LCD·éç»
#define LCD_HEIGH         600    // LCD·éç»

#define LCD_FCLK          LTDC_CLK_48MHZ // LCD¶é¤æé¢¤æ: LTDC_CLK_32MHZ/LTDC_CLK_48MHZ/LTDC_CLK_64MHZ/LTDC_CLK_12MHZ
                                         // ä¸·ç·åç¢ä½¿éè¾: 800*480->LTDC_CLK_32MHZ; 1024*600->LTDC_CLK_48MHZ;
                                         //                           480*272->LTDC_CLK_12MHZ;  
#define LCD_HSYNC_WIDTH   20     // HSYNC·å·é¤æ
#define LCD_HBP_WIDTH     140    // HBP·å·é¤æ
#define LCD_HFP_WIDTH     160    // HFP·å·é¤æ

#define LCD_VSYNC_WIDTH   3      // VSYNC·å·é¤æ
#define LCD_VBP_WIDTH     20     // VBP·å·é¤æ
#define LCD_VFP_WIDTH     12     // VFP·å·é¤æ
#endif

        
// ·é¤æ·ç¡··é¤æ
#define LCD_HSYNC_POLARIRY    0   // HSYNC·é¤æ·é¤æ: 0(ç¢å¹)1(ç¢å¹)
#define LCD_VSYNC_POLARIRY    0   // VSYNC·é¤æ·é¤æ: 0(ç¢å¹)1(ç¢å¹)
#define LCD_DE_POLARIRY       0   // DE·é¤æ·é¤æ: 0(ç¢å¹)1(ç¢å¹)
#define LCD_CLK_POLARIRY      0   // Pixel Clock·é¤æ·é¤æ: 0(ç¢å¹)1(ç¢å¹)

// ·é·é¤æ·è
#define LCD_BACKCOLOR_RED     0
#define LCD_BACKCOLOR_GREEN   0
#define LCD_BACKCOLOR_BLUE    100

//--------------------------------------------------------------------------------
// ·é¤ææ¯¤æ¾éå§·é
#define LTDC_BUFFER_SIZE      0x00200000

// ¾é¤æ1·é¤æ
#define LTDC_LAYER1_EN       1  //LTDC¾é¤æ1ä½¿é¤æ·é¤æ: 1, ä½¿é¤æ;  0, ;         
#if (LTDC_LAYER1_EN > 0)
#define LTDC_L1_WINDOW_X0    0     // ·é¤æ¾é¤ææ°´å¹³·å,·å0x0000xFFF
#define LTDC_L1_WINDOW_X1    LCD_WIDTH   // ·é¤æ¾é¤ææ°´å¹³·é¤æä½¤æ,·å0x0000xFFF
#define LTDC_L1_WINDOW_Y0    0     // ·é¤æ¾éå§´é¤æå§,·å0x0000x7FF
#define LTDC_L1_WINDOW_Y1    LCD_HEIGH   // ·é¤æ¾éå§´é¤æ·ä,·å0x0000x7FF    
    
#define LTDC_L1_PIXEL_FORMAT  LTDC_PIXEL_FORMAT_RGB565 // ·é¤æ¾é¤æ·ä½¿ç¢·è·å
    
#define LTDC_L1_ALPHA       0xFF       // Alpha·é¤æ,·å0x00´º0xFF
    
// ·é¤æ¾é¤æ·é¤æ 
#define LTDC_L1_BF1       LTDC_BF1_PAxCA  // ·éä¼·é¤æ1: LTDC_BF1_CATDC_BF1_PAxCA
#define LTDC_L1_BF2       LTDC_BF2_PAxCA // ·éä¼·é¤æ2: LTDC_BF2_CATDC_BF2_PAxCA
    
#define LTDC_L1_IMAGE_WIDTH      LCD_WIDTH     // ·é¤æ·èå¸§é¤æ·é¤æç­¹æ·é¤æè¦¤æè¾¾æ¥ºä¼æ¢°æ·é·éè½ 0x0000  0x1FFF
#define LTDC_L1_IMAGE_HEIGH      LCD_HEIGH     // ·é¤æ·èå¸§é¤æ·é¤æ·é¤æ·é¤æè¦¤æè¾¾æ¥º·é¤æ·é¤æ½¿ 0x000  0xFFF 
#define LTDC_L1_START_ADDRESS    (SDRAM_LCD_START_ADDR) // ¾é¤æ·æ·å

// ·é¤æ¾é¤æé»¤æ
#define LTDC_L1_ALPHA0            0      // ·é¤æ¾é¤æé»¤æAlpha,·å0x00´º0xFF,·å¸æ·åBackcolorä¸·ä½¿,·é ARGB ·è·å·é¤æ¾éå§·è
#define LTDC_L1_BACKCOLOR_RED     255     
#define LTDC_L1_BACKCOLOR_GREEN   0
#define LTDC_L1_BACKCOLOR_BLUE    0    
#endif

//--------------------------------------------------------------------------------
// ¾é¤æ2·é¤æ
#define LTDC_LAYER2_EN       0  //LTDC¾é¤æ2ä½¿é¤æ·é¤æ: 1, ä½¿é¤æ;  0, ;         
#if (LTDC_LAYER2_EN > 0)
#define LTDC_L2_WINDOW_X0    0     // ·é¤æ¾é¤ææ°´å¹³·å,·å0x0000xFFF
#define LTDC_L2_WINDOW_X1    LCD_WIDTH   // ·é¤æ¾é¤ææ°´å¹³·é¤æä½¤æ,·å0x0000xFFF
#define LTDC_L2_WINDOW_Y0    0     // ·é¤æ¾éå§´é¤æå§,·å0x0000x7FF
#define LTDC_L2_WINDOW_Y1    LCD_HEIGH   // ·é¤æ¾éå§´é¤æ·ä,·å0x0000x7FF    
    
#define LTDC_L2_PIXEL_FORMAT  LTDC_PIXEL_FORMAT_RGB565 // ·é¤æ¾é¤æ·ä½¿ç¢·è·å
    
#define LTDC_L2_ALPHA        0xFF    // Alpha·é¤æ,·å0x00´º0xFF
    
// ·é¤æ¾é¤æ·é¤æ 
#define LTDC_L2_BF1      LTDC_BF1_PAxCA  // ·éä¼·é¤æ1: LTDC_BF1_CATDC_BF1_PAxCA
#define LTDC_L2_BF2      LTDC_BF2_PAxCA  // ·éä¼·é¤æ2: LTDC_BF2_CATDC_BF2_PAxCA
    
#define LTDC_L2_IMAGE_WIDTH      LCD_WIDTH     // ·é¤æ·èå¸§é¤æ·é¤æç­¹æ·é¤æè¦¤æè¾¾æ¥ºä¼æ¢°æ·é·éè½ 0x0000  0x1FFF
#define LTDC_L2_IMAGE_HEIGH      LCD_HEIGH     // ·é¤æ·èå¸§é¤æ·é¤æ·é¤æ·é¤æè¦¤æè¾¾æ¥º·é¤æ·é¤æ½¿ 0x000  0xFFF 
#define LTDC_L2_START_ADDRESS    (SDRAM_LCD_START_ADDR+LTDC_BUFFER_SIZE)   // ¾é¤æ·æ·å

// ·é¤æ¾é¤æé»¤æ
#define LTDC_L2_ALPHA0            0      // ·é¤æ¾é¤æé»¤æAlpha,·å0x00´º0xFF,·å¸æ·åBackcolorä¸·ä½¿,·é ARGB ·è·å·é¤æ¾éå§·è
#define LTDC_L2_BACKCOLOR_RED     0     
#define LTDC_L2_BACKCOLOR_GREEN   0
#define LTDC_L2_BACKCOLOR_BLUE    0    
#endif

#endif

/************************************************************************************/
// DMA2D·é¤æ
/************************************************************************************/
#define DMA2D_EN                     0      // 1, ä½¿é¤æ;  0, ;

#if (DMA2D_EN > 0)
// ·é¤æ·æ·é¤æ
#define DMA2D_TIMEOUT                100    // ·éè¯§æ·é¤æ: ·äms; 

// è®¹æä½¿é¤æ·é¤æ
#define DMA2D_INT_EN                 0      // ·é¤æè®¹æä½¿é¤æ: 0x3F(DMA2D_INT_MASK),ä½¿é¤æ¨é¤æè®¹æ; 0,·é¤æè®¹æ;

// ·é¤æ·é¤æ·é¤æ·é¤æ
#define DMA2D_NO_BLOCK_EN            0      // ·é´æ·é¤æ·é¤æ·é¤æ: 1, ä½¿é¤æ; 0, 

// ·é¤æ·é¤æ·é¤æ
#define DMA2D_OUT_COLOR_FORMAT       2      // ·é¤æ·é¤æ¼´·å: 0(DMA2D_FORMAT_ARGB8888); 1(DMA2D_FORMAT_RGB888); 2(DMA2D_FORMAT_RGB565); 
                                            // 3(DMA2D_FORMAT_ARGB1555); 4(DMA2D_FORMAT_ARGB4444);                                   
#define DMA2D_OUT_ALPHA_INVERTED_EN  0      // ·é¤æ·éç´PHA¤æä½¿é¤æ: 0(DMA2D_ALPHA_REGULAR), ·é¤æ·é; 1(DMA2D_ALPHA_INVERTED), ·è½¬·é; 
#define DMA2D_OUT_RB_SWAP_EN         0      // ·é¤æ·é¤æ¼´·å¤æ¤æ¾¥¤æ·ä½¿: 0(DMA2D_REGULAR_RB), ·é¤æ(RGB or ARGB); 1(DMA2D_SWAP_RB), ·é¤æ(BGR or ABGR); 

// ¤æ(·é¤æ)·é¤æ·é¤æ
#define DMA2D_FG_COLOR_FORMAT        2      // ·é¤æ¤æ·é¤æ²é¤æå¼: 0(DMA2D_FORMAT_ARGB8888); 1(DMA2D_FORMAT_RGB888); 2(DMA2D_FORMAT_RGB565); 
                                            // 3(DMA2D_FORMAT_ARGB1555); 4(DMA2D_FORMAT_ARGB4444);5(DMA2D_FORMAT_L8);6(DMA2D_FORMAT_AL44);
                                            // 7(DMA2D_FORMAT_AL88); 8(DMA2D_FORMAT_L4);9(DMA2D_FORMAT_A8);10(DMA2D_FORMAT_A4);11(DMA2D_FORMAT_YCBCR);
#define DMA2D_FG_ALPHA_MODE          0      // ·é¤æ¤ælphaæ¨¡å: 0(DMA2D_ALPHA_MODE_NONE),·é©æ¤æ´¥·å·éçµpha¤æ; 
                                            //                      1(DMA2D_ALPHA_MODE_REPLACE), ·éæ´¥è¾é¤ælpha¤æ¼é¥æä¸LPHA[7:0]; 
                                            //                      2(DMA2D_ALPHA_MODE_MIX), ·éæ´¥è¾é¤ælpha¤æ¼é¥æä¸LPHA[7:0]·åå§lpha¤æ¼éä¾¥ää¼; 
#define DMA2D_FG_ALPHA_INVERTED_EN   0      // ·é¤æ¤æLPHA¤æä½¿é¤æ: 0(DMA2D_ALPHA_REGULAR), ·é¤æ·é; 1(DMA2D_ALPHA_INVERTED), ·è½¬·é; 
#define DMA2D_FG_RB_SWAP_EN          0      // ·é¤æ¤æ·é¤æ²é¤æå¼¤æR¤æ¤æ¾¥¤æ·ä½¿: 0(DMA2D_REGULAR_RB), ·é¤æ(RGB or ARGB); 1(DMA2D_SWAP_RB), ·é¤æ(BGR or ABGR); 
#define DMA2D_FG_YCbCr_CSS           0      // ·é¤æ¤æ·é¤æCbCr·èæ¨¡å¾¥è¯§æ·é¤æå¼: 0(DMA2D_YCbCr_CSS_444), 4:4:4; 1(DMA2D_YCbCr_CSS_422), 4:2:2; 2(DMA2D_YCbCr_CSS_420), 4:2:0; 
#define DMA2D_FG_CLUT_SIZE           256    // ·é¤æ¤æ·èCLUT·å: 1-256
#define DMA2D_FG_CLUT_CORLOR_MODE    0      // ·é¤æ¤æ²é¤æCLUT·èæ¨¡å: 0(DMA2D_CCM_ARGB8888)RGB8888; 1(DMA2D_CCM_RGB888)GB888;

// ·é¤æ(·é¤æ)·é¤æ·é¤æ
#define DMA2D_BG_COLOR_FORMAT        2      // ·é·é¤æ·è·å: 0(DMA2D_FORMAT_ARGB8888); 1(DMA2D_FORMAT_RGB888); 2(DMA2D_FORMAT_RGB565); 
                                            // 3(DMA2D_FORMAT_ARGB1555); 4(DMA2D_FORMAT_ARGB4444);5(DMA2D_FORMAT_L8);6(DMA2D_FORMAT_AL44);
                                            // 7(DMA2D_FORMAT_AL88); 8(DMA2D_FORMAT_L4);9(DMA2D_FORMAT_A8);10(DMA2D_FORMAT_A4);11(DMA2D_FORMAT_YCBCR);
#define DMA2D_BG_ALPHA_MODE          0      // ·é·é¤æAlphaæ¨¡å: 0(DMA2D_ALPHA_MODE_NONE),·é©æ¤æ´¥·å·éçµpha¤æ; 
                                            //                      1(DMA2D_ALPHA_MODE_REPLACE), ·éæ´¥è¾é¤ælpha¤æ¼é¥æä¸LPHA[7:0]; 
                                            //                      2(DMA2D_ALPHA_MODE_MIX), ·éæ´¥è¾é¤ælpha¤æ¼é¥æä¸LPHA[7:0]·åå§lpha¤æ¼éä¾¥ää¼; 
#define DMA2D_BG_ALPHA_INVERTED_EN   0      // ·é·é¤æALPHA¤æä½¿é¤æ: 0(DMA2D_ALPHA_REGULAR), ·é¤æ·é; 1(DMA2D_ALPHA_INVERTED), ·è½¬·é; 
#define DMA2D_BG_RB_SWAP_EN          0      // ·é·é¤æ·è·å¤æ¤æ¾¥¤æ·ä½¿: 0(DMA2D_REGULAR_RB), ·é¤æ(RGB or ARGB); 1(DMA2D_SWAP_RB), ·é¤æ(BGR or ABGR); 
#define DMA2D_BG_YCbCr_CSS           0      // ·é·é¤æ·èYCbCr·èæ¨¡å¾¥è¯§æ·é¤æå¼: 0(DMA2D_YCbCr_CSS_444), 4:4:4; 1(DMA2D_YCbCr_CSS_422), 4:2:2; 2(DMA2D_YCbCr_CSS_420), 4:2:0; 
#define DMA2D_BG_CLUT_SIZE           256    // ·é·é¤æLUT·å: 1-256
#define DMA2D_BG_CLUT_CORLOR_MODE    0      // ·é·èLUT·èæ¨¡å: 0(DMA2D_CCM_ARGB8888)RGB8888; 1(DMA2D_CCM_RGB888)GB888;

#endif

/************************************************************************************/
// JPEG·é¤æ
/************************************************************************************/
#define JPEG_EN                      0      // 1, ä½¿é¤æ;  0, ;

#if (JPEG_EN > 0)

#endif
/************************************************************************************/
// MDMA·é¤æ
/************************************************************************************/
//#include "mdma_config.h"  // mdma_config.h ÎÄ¼þ²»´æÔÚ, ÔÝÊ±¹Ø±Õ

/************************************************************************************/
// LVGL·é¤æ
/************************************************************************************/
//#include "lvgl_config.h"  // ÒÑ¹Ø±Õ LVGL, ²»ÐèÒª¸ÃÍ·ÎÄ¼þ

/***********************************************************************************/

#endif //#ifndef __AMKN8639_CONFIG_H 