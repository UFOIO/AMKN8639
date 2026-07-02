path = r"C:\Users\gjt\Desktop\项目工程\2026内蒙轻型机库项目\软件\STMMY-H743MCU\AMKNxxxx_FreeRTOS_V1.31.02_20260501\targets\AMKN8639\BL_BUILD\src\bootloader.c"
with open(path, "r") as f:
    code = f.read()

old1 = "static int call_from_sram(void* fn, uint32_t arg1, uint32_t arg2, int is_write) {"
new1 = """static int call_from_sram(void* fn, uint32_t arg1, uint32_t arg2, int is_write) {
    uint8_t* src = (uint8_t*)fn;
    uint8_t* dst = (uint8_t*)SRAM_BASE;
    unsigned int offset = (unsigned int)fn & 0xFFF;
    for(int i=0;i<2048;i++)dst[i]=src[i];
    __asm("dsb"); __asm("isb");
    if(is_write){
        return ((write_fn_t)((SRAM_BASE + offset) | 1))(arg1, arg2);
    } else {
        return ((erase_fn_t)((SRAM_BASE + offset) | 1))(arg1);
    }
}"""

# Find the function and replace it
start = code.find(old1)
if start < 0:
    print("ERROR: call_from_sram not found")
    exit(1)
# Find the matching closing brace
depth = 0
end = start
for i in range(start, len(code)):
    if code[i] == '{': depth += 1
    elif code[i] == '}':
        depth -= 1
        if depth == 0:
            end = i + 1
            break
old_func = code[start:end]
code = code.replace(old_func, new1)

old2 = "static int do_bl_upgrade_sram(void){"
new2 = """static int do_bl_upgrade_sram(void){
    uint8_t*s=(uint8_t*)bl_upgrade_sram;
    uint8_t*d=(uint8_t*)SRAM_BASE;
    unsigned int offset=(unsigned int)bl_upgrade_sram&0xFFF;
    for(int i=0;i<4096;i++)d[i]=s[i];
    __asm("dsb");__asm("isb");
    return ((int(*)(void))((SRAM_BASE+offset)|1))();
}"""

start2 = code.find(old2)
depth2 = 0
end2 = start2
for i in range(start2, len(code)):
    if code[i] == '{': depth2 += 1
    elif code[i] == '}':
        depth2 -= 1
        if depth2 == 0:
            end2 = i + 1
            break
old_func2 = code[start2:end2]
code = code.replace(old_func2, new2)

with open(path, "w", newline="\n") as f:
    f.write(code)
print("Done - byte copy + offset fix")
