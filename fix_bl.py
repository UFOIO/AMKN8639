path = r"C:\Users\gjt\Desktop\项目工程\2026内蒙轻型机库项目\软件\STMMY-H743MCU\AMKNxxxx_FreeRTOS_V1.31.02_20260501\targets\AMKN8639\BL_BUILD\src\bootloader.c"
with open(path, "r") as f:
    code = f.read()

code = code.replace("#define GPIOB ((Gpio*)0x58020400)", "#define GPIOG ((Gpio*)0x58021800)")
code = code.replace(
    "void led_init(void){RCC_AHB4ENR|=2;GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14);GPIOB->ODR|=(1<<7);}",
    "void buzz_init(void){RCC_AHB4ENR|=1<<6;GPIOG->MODER&=~(3<<6);GPIOG->MODER|=(1<<6);}")
code = code.replace(
    "void led_on(void){GPIOB->ODR&=~(1<<7);}",
    "void buzz_on(void){GPIOG->BSRR=(1<<3);}")
code = code.replace(
    "void led_off(void){GPIOB->ODR|=(1<<7);}",
    "void buzz_off(void){GPIOG->BSRR=(1<<19);}")
code = code.replace(
    "#define SRAM_BASE",
    "void buzz_beep(int ms){buzz_on();dly_ms(ms);buzz_off();}\n#define SRAM_BASE")
code = code.replace("led_init()", "buzz_init()")
code = code.replace("led_on()", "buzz_on()")
code = code.replace("led_off()", "buzz_off()")
code = code.replace("GPIOB->ODR^=0x80", "buzz_beep(30)")
code = code.replace("V7.1", "V16")
code = code.replace('"LED\\n"', '"BEEP\\n"')
code = code.replace("GPIOB", "GPIOG")

with open(path, "w", newline="\n") as f:
    f.write(code)
print("Done")
