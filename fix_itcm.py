import re
path = r"C:\Users\gjt\Desktop\项目工程\2026内蒙轻型机库项目\软件\STMMY-H743MCU\AMKNxxxx_FreeRTOS_V1.31.02_20260501\targets\AMKN8639\BL_BUILD\src\bootloader.c"
with open(path, "rb") as f:
    data = f.read()

dollar = ord("$")
underscore = ord("_")

# Fix: replace $$ with __ everywhere first (undo damage)
fixed = bytearray()
i = 0
while i < len(data):
    if i+1 < len(data) and data[i] == dollar and data[i+1] == dollar:
        fixed.append(underscore)
        fixed.append(underscore)
        i += 2
    else:
        fixed.append(data[i])
        i += 1

# Now put $$ back only for ER_ITCM symbols
def replace_pattern(ba, old, new):
    result = bytearray()
    i = 0
    while i < len(ba):
        if ba[i:i+len(old)] == old:
            result.extend(new)
            i += len(old)
        else:
            result.append(ba[i])
            i += 1
    return result

fixed = replace_pattern(fixed, b"Load__ER_ITCM__Base", b"Load$$ER_ITCM$$Base")
fixed = replace_pattern(fixed, b"Image__ER_ITCM__Base", b"Image$$ER_ITCM$$Base")
fixed = replace_pattern(fixed, b"Image__ER_ITCM__Length", b"Image$$ER_ITCM$$Length")

with open(path, "wb") as f:
    f.write(fixed)

# Verify
import re
with open(path, "r") as f:
    for line in f:
        if "ITCM" in line and ("Base" in line or "Length" in line):
            print(line.rstrip())
print("Done")
