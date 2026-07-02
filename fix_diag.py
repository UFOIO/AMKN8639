path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()
old_diag = b'ups("\\\\n");'
new_diag = b'ups("\\n");'
print('count old:', data.count(old_diag))
data = data.replace(old_diag, new_diag, 1)
idx = data.find(b'DBG CR1=')
print(data[idx:idx+500].decode('utf-8', errors='replace'))
with open(path, 'wb') as f:
    f.write(data)
print('OK, size:', len(data))
