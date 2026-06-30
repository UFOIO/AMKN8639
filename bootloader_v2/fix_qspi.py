path = r"C:\Users\gjt\Desktop\????\2026????????\??\STMMY-H743MCU\AMKNxxxx_FreeRTOS_V1.31.02_20260501\targets\AMKN8639\bootloader_v2\bl_v5.c"
with open(path, "r") as f:
    content = f.read()

# Find qspi_jedec_id function and replace
old = "static uint32_t qspi_jedec_id(void) {"
new = """static void qspi_send_cmd(uint8_t cmd) {
    GPIOG_BSRR = (1<<22);
    QSPI_CCR = cmd | (1<<8) | (0<<26) | (1<<24);
    QSPI_DLR = 0;
    QSPI_AR = 0;
    QSPI_CR = 1;
    QSPI_CR = 0;
    GPIOG_BSRR = (1<<6);
}

static void qspi_reset_flash(void) {
    qspi_send_cmd(0x66);
    qspi_send_cmd(0x99);
    {volatile int i=3000; while(i--) __asm("nop");}
    qspi_send_cmd(0xAB);
    {volatile int i=3000; while(i--) __asm("nop");}
}

static uint32_t qspi_jedec_id(void) {"""

content = content.replace(old, new)

# Add flash reset call before delay
old2 = '/* Power-up delay for W25Q64 */'
new2 = '/* Reset flash from QPI to SPI mode */\n    qspi_reset_flash();\n\n    /* Power-up delay for W25Q64 */'
content = content.replace(old2, new2)

with open(path, "w") as f:
    f.write(content)
print("Done")
