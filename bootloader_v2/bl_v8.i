#line 1 "bootloader_v2/bl_v8.c"
 
#line 1 "D:\\keil5\\ARM\\ARMCC\\bin\\..\\include\\stdint.h"
 
 





 









     
#line 27 "D:\\keil5\\ARM\\ARMCC\\bin\\..\\include\\stdint.h"
     











#line 46 "D:\\keil5\\ARM\\ARMCC\\bin\\..\\include\\stdint.h"





 

     

     
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;
typedef   signed       __int64 int64_t;

     
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;
typedef unsigned       __int64 uint64_t;

     

     
     
typedef   signed          char int_least8_t;
typedef   signed short     int int_least16_t;
typedef   signed           int int_least32_t;
typedef   signed       __int64 int_least64_t;

     
typedef unsigned          char uint_least8_t;
typedef unsigned short     int uint_least16_t;
typedef unsigned           int uint_least32_t;
typedef unsigned       __int64 uint_least64_t;

     

     
typedef   signed           int int_fast8_t;
typedef   signed           int int_fast16_t;
typedef   signed           int int_fast32_t;
typedef   signed       __int64 int_fast64_t;

     
typedef unsigned           int uint_fast8_t;
typedef unsigned           int uint_fast16_t;
typedef unsigned           int uint_fast32_t;
typedef unsigned       __int64 uint_fast64_t;

     




typedef   signed           int intptr_t;
typedef unsigned           int uintptr_t;


     
typedef   signed     long long intmax_t;
typedef unsigned     long long uintmax_t;




     

     





     





     





     

     





     





     





     

     





     





     





     

     






     






     






     

     


     


     


     

     
#line 216 "D:\\keil5\\ARM\\ARMCC\\bin\\..\\include\\stdint.h"

     



     






     
    
 



#line 241 "D:\\keil5\\ARM\\ARMCC\\bin\\..\\include\\stdint.h"

     







     










     











#line 305 "D:\\keil5\\ARM\\ARMCC\\bin\\..\\include\\stdint.h"






 
#line 3 "bootloader_v2/bl_v8.c"
#line 1 "bootloader_v2/flash_map.h"



 



#line 9 "bootloader_v2/flash_map.h"

 





 



 



 



 



 



 



 



 



 



 
typedef struct __attribute__((packed)) {
    uint32_t magic;           
    uint32_t boot_status;     
    uint32_t slot_b_size;     
    uint32_t sec_counter;     
    uint8_t  new_sha256[32];  
    uint32_t boot_attempts;   
    uint32_t crc32;           
} ota_status_t;







 
typedef enum {
    BOOT_SLOT_A = 0,
    BOOT_SWAP_B_TO_A,
    BOOT_NO_APP,
} boot_decision_t;

#line 4 "bootloader_v2/bl_v8.c"

#line 25 "bootloader_v2/bl_v8.c"

#line 34 "bootloader_v2/bl_v8.c"






static void led_on(void){(*(volatile uint32_t*)0x58022018)=(1<<8);}
static void led_off(void){(*(volatile uint32_t*)0x58022018)=(1<<24);}
static void dly(volatile uint32_t n){while(n--){};}
static void blip(int ms){led_on();dly((uint32_t)ms*64000);led_off();}
static void blink(int n,int ms){int i;for(i=0;i<n;i++){blip(ms);dly((uint32_t)ms*64000);}}

static void upc(char c){volatile int t=1000000;while(!((*(volatile uint32_t*)0x4001101C)&(1<<7))&&--t){}(*(volatile uint32_t*)0x40011028)=c;}
static void ups(const char *s){while(*s)upc(*s++);}
static void uph(uint32_t v){static const char h[]="0123456789ABCDEF";int i;for(i=28;i>=0;i-=4)upc(h[(v>>i)&0xF]);}
static void upp(const char *s,uint32_t v){ups(s);uph(v);ups("\r\n");}
static int urx(void){return((*(volatile uint32_t*)0x4001101C)&(1<<5))!=0;}

__asm void jump(uint32_t sp,uint32_t pc){MSR MSP,r0;BX r1;}

static uint32_t ct[256];
static void crcinit(void){uint32_t i;for(i=0;i<256;i++){uint32_t c=i;int j;for(j=0;j<8;j++)c=(c>>1)^((c&1)?0xEDB88320:0);ct[i]=c;}}

static void qinit(void){
    volatile uint32_t r;
     
    (*(volatile uint32_t*)0x52005000)=0;dly(100);
    (*(volatile uint32_t*)0x52005004)=(0<<0)|(2<<8)|(22<<16);  
     
    (*(volatile uint32_t*)0x52005000)=(3<<24)|(1<<9)|(1<<7)|(1<<6)|(1<<0);  
    dly(1000);
    upp("CR=",(*(volatile uint32_t*)0x52005000));upp("DCR=",(*(volatile uint32_t*)0x52005004));upp("SRi=",(*(volatile uint32_t*)0x52005008));upp("AHB3=",(*(volatile uint32_t*)0x580244D4));
}

static uint32_t jid(void){
    uint32_t id=0,sr;
    int i;

     
    for(i=0;i<32&&((*(volatile uint32_t*)0x52005008)&(1<<2));i++){volatile uint32_t d=(*(volatile uint32_t*)0x52005020);(void)d;}
    (*(volatile uint32_t*)0x5200500C)=0x1F;
    upp("SRa=",(*(volatile uint32_t*)0x52005008));

     
    (*(volatile uint32_t*)0x52005014)=0x9F|(1<<8)|(1<<18)|(1<<24)|(1<<26);
    __asm("dsb");
    upp("CCR=",(*(volatile uint32_t*)0x52005014));

     
    (*(volatile uint32_t*)0x52005018)=0;
    __asm("dsb");
    (*(volatile uint32_t*)0x52005010)=2;
    __asm("dsb");
    upp("DLR=",(*(volatile uint32_t*)0x52005010));
    upp("SRb=",(*(volatile uint32_t*)0x52005008));

     
    {volatile int to=5000000;
    while(to--){
        sr=(*(volatile uint32_t*)0x52005008);
        if(sr&(1<<1))break;
        if(sr&(1<<0)){upp("TE=",sr);(*(volatile uint32_t*)0x5200500C)=1;break;}
    }
    if(to<=0)upp("TO=",(*(volatile uint32_t*)0x52005008));}

    upp("SRc=",(*(volatile uint32_t*)0x52005008));
    if((*(volatile uint32_t*)0x52005008)&(1<<1)){
        uint8_t b0=(uint8_t)(*(volatile uint32_t*)0x52005020),b1=(uint8_t)(*(volatile uint32_t*)0x52005020),b2=(uint8_t)(*(volatile uint32_t*)0x52005020);
        id=((uint32_t)b0<<16)|((uint32_t)b1<<8)|b2;
    }
    upp("ID=",id);
    return id;
}

static ota_status_t ota;
static int ota_rd(void){
    const uint32_t*s=(const uint32_t*)0x081A0000;
    uint32_t*d=(uint32_t*)&ota;
    {int _i;for(_i=0;_i<sizeof(ota_status_t)/4;_i++)d[i]=s[i];
    return(ota.magic==0x4F544148)?0:-1;
}

__attribute__((noreturn))
void Reset_Handler(void){
    while(!((*(volatile uint32_t*)0x58024400)&(1<<2))){}
    (*(volatile uint32_t*)0x52002000)=0x04;

    (*(volatile uint32_t*)0x580244E0)|=(1<<8);(*(volatile uint32_t*)0x58022000)&=~(3<<16);(*(volatile uint32_t*)0x58022000)|=(1<<16);
    blink(3,30);dly(640000);

    (*(volatile uint32_t*)0x580244E0)|=(1<<0);(*(volatile uint32_t*)0x580244F0)|=(1<<4);
    (*(volatile uint32_t*)0x58020000)&=~((3<<18)|(3<<20));(*(volatile uint32_t*)0x58020000)|=(2<<18)|(2<<20);
    (*(volatile uint32_t*)0x58020008)|=(3<<18)|(3<<20);(*(volatile uint32_t*)0x58020024)&=~(0xFF<<4);(*(volatile uint32_t*)0x58020024)|=(7<<4)|(7<<8);
    (*(volatile uint32_t*)0x4001100C)=64000000/115200;(*(volatile uint32_t*)0x40011000)=(1<<3)|(1<<2)|(1<<0);

    ups("\r\n=== V8 ===\r\n");crcinit();

    (*(volatile uint32_t*)0x580244D4)|=(1<<14);(*(volatile uint32_t*)0x580244E0)|=(1<<5)|(1<<6);
    (*(volatile uint32_t*)0x58021400)&=~((3<<16)|(3<<18)|(3<<20));
    (*(volatile uint32_t*)0x58021400)|=((2<<16)|(2<<18)|(2<<20));
    (*(volatile uint32_t*)0x58021408)|=(3<<16)|(3<<18)|(3<<20);
    (*(volatile uint32_t*)0x5802140C)&=~((3<<16)|(3<<18)|(3<<20));
    (*(volatile uint32_t*)0x5802140C)|=(1<<16)|(1<<18)|(1<<20);
    (*(volatile uint32_t*)0x58021420)&=~0xFF;(*(volatile uint32_t*)0x58021420)|=(9<<0)|(9<<4);
    (*(volatile uint32_t*)0x58021424)&=~(0xF<<8);(*(volatile uint32_t*)0x58021424)|=(9<<8);
    (*(volatile uint32_t*)0x58021800)&=~(3<<12);(*(volatile uint32_t*)0x58021800)|=(2<<12);
    (*(volatile uint32_t*)0x58021808)|=(3<<12);(*(volatile uint32_t*)0x5802180C)|=(1<<12);
    (*(volatile uint32_t*)0x58021824)&=~(0xF<<8);(*(volatile uint32_t*)0x58021824)|=(9<<8);

    qinit();

    uint32_t jd=0;
    {int _r;for(_r=0;_r<3;_r++){
        upp("Try",r);
        jd=jid();
        if(jd&&jd!=0xFFFFFF)break;
        dly(640000);
    }

    if(jd&&jd!=0xFFFFFF){ups("Flash OK\r\n");blink(2,50);}
    else{ups("Flash FAIL\r\n");blink(5,20);dly(640000);}

    ups("OTA: ");if(ota_rd()==0)uph(ota.boot_status);else ups("none");ups("\r\n");

    uint32_t sp=*(volatile uint32_t*)0x08020000;
    uint32_t pc=*(volatile uint32_t*)(0x08020000+4);
    upp("SP=",sp);upp("PC=",pc);

    if(sp>=0x20000000&&sp<0x20020000&&pc>=0x08020000&&pc<(0x08020000+1536*1024)){
        ups("Boot\r\n");blip(150);dly(320000);
        (*(volatile uint32_t*)0x52005000)&=~1;(*(volatile uint32_t*)0x40011000)=0;
        jump(sp,pc);
    }
    ups("NO APP\r\n");
    while(1){led_on();dly(6400000);led_off();dly(6400000);}
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){led_off();dly(2000000);led_on();dly(2000000);}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={0x2001FFC0,(uint32_t)Reset_Handler,(uint32_t)NMI_Handler,(uint32_t)HardFault_Handler,(uint32_t)MemManage_Handler,(uint32_t)BusFault_Handler,(uint32_t)UsageFault_Handler,0,0,0,0,0,0,0,0,(uint32_t)SysTick_Handler};

