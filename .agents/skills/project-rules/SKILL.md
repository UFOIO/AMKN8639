---
name: project-rules
description: AMKN8639 项目专属安全规则——记录 2026-06-28 文件损坏事故，禁止一切可能导致源码损坏的操作
---

# AMKN8639 项目安全规则

## 事故记录

**时间**：2026-06-28 00:00 左右
**文件**：source/user_app.c（176KB / 1740行，GB2312 编码）
**原因**：PowerShell Set-Content -NoNewline 去掉了全部 \r\n 换行符
**后果**：文件变成一整行（173KB），编译失败，耗时数小时才通过恢复工具还原
**根本原因**：Get-Content | -replace | Set-Content -NoNewline 管道中 -NoNewline 不会在行间插入分隔符

## 铁律

### 1. 禁止使用 Set-Content -NoNewline 处理源码文件

- 禁止对 .c、.h、.s、.md 等文本文件使用 Set-Content -NoNewline
- 禁止在任何可能修改源码文件的 PowerShell 管道中使用 -NoNewline 开关

### 2. 修改文件的正确方式

- **优先**：Python open(path, 'wb').write(data) —— 保持原始编码不变
- **次选**：PowerShell Out-File -Encoding utf8 -NoNewline（仅当明确知道文件是 UTF-8 时）
- **禁止**：Set-Content 用于修改已有文件内容

### 3. 编码意识

- 本项目 user_app.c 使用 **GB2312/ANSI** 编码（非 UTF-8）
- 任何文本处理前先检查字节内容，而不是假设编码
- Python 读取时使用 open(path, 'rb').read() 再根据内容判断编码

### 4. 修改前必须备份

`
copy source\user_app.c source\user_app.c.bak
`

### 5. Git 纪律

- 每次编译通过后立即提交：git add -A && git commit -m "..."
- 不要在未验证编译的情况下提交
- source/user_app.c 是核心文件，任何修改都要审慎

## 工具使用约束

| 操作 | 允许 | 禁止 |
|------|------|------|
| 读取文件 | Get-Content、Python open | - |
| 写入文件 | Python open(f,'wb').write() | Set-Content -NoNewline |
| 搜索文本 | g、Select-String | - |
| 修改文件 | pply_patch 工具 | PowerShell 管道直接覆盖 |
