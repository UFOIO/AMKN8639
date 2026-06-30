#!/usr/bin/env python3
"""AMKN8639 BL Build Tool - 简单上位机"""
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import subprocess, os, sys, io, threading

# Fix console encoding
if sys.platform == "win32":
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

# ---- 默认路径 (按你的环境改) ----
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BL_DIR = os.path.join(BASE_DIR, "bootloader")
APP_DEFAULT = os.path.join(BASE_DIR, "output", "bin", "AMKN8639_APP.bin")
SX_DEFAULT = os.path.join(BASE_DIR, "output", "bin", "AMKN8639_SX.bin")
KEIL_UV4 = r"D:\keil5\UV4\UV4.exe"
KEIL_PROJ = os.path.join(BASE_DIR, "AMKN8639.uvprojx")
STM32_PROG = r"D:\stmp\bin\STM32_Programmer_CLI.exe"

class BuildTool:
    def __init__(self, root):
        self.root = root
        root.title("AMKN8639 BL Build Tool")
        root.geometry("640x520")
        root.resizable(True, True)

        # ---- 文件选择 ----
        frm = tk.LabelFrame(root, text="路径设置", padx=8, pady=4)
        frm.pack(fill="x", padx=8, pady=4)

        tk.Label(frm, text="BL 源文件:").grid(row=0, column=0, sticky="w")
        self.bl_var = tk.StringVar(value=os.path.join(BL_DIR, "bootloader.c"))
        tk.Entry(frm, textvariable=self.bl_var, width=60).grid(row=0, column=1, padx=4)
        tk.Button(frm, text="...", command=lambda: self._browse(self.bl_var, "*.c")).grid(row=0, column=2)

        tk.Label(frm, text="APP.bin:").grid(row=1, column=0, sticky="w", pady=2)
        self.app_var = tk.StringVar(value=APP_DEFAULT)
        tk.Entry(frm, textvariable=self.app_var, width=60).grid(row=1, column=1, padx=4)
        tk.Button(frm, text="...", command=lambda: self._browse(self.app_var, "*.bin")).grid(row=1, column=2)

        tk.Label(frm, text="输出 SX.bin:").grid(row=2, column=0, sticky="w", pady=2)
        self.sx_var = tk.StringVar(value=SX_DEFAULT)
        tk.Entry(frm, textvariable=self.sx_var, width=60).grid(row=2, column=1, padx=4)

        # ---- 按钮 ----
        btn_frm = tk.Frame(root)
        btn_frm.pack(fill="x", padx=8, pady=4)

        self.btn_build = tk.Button(btn_frm, text="1. 编译 BL + 打包 SX.bin", width=22,
                                   command=self._build_bl, bg="#4CAF50", fg="white")
        self.btn_build.pack(side="left", padx=2)

        self.btn_app = tk.Button(btn_frm, text="2. 编译 APP", width=14,
                                 command=self._build_app, bg="#2196F3", fg="white")
        self.btn_app.pack(side="left", padx=2)

        self.btn_flash = tk.Button(btn_frm, text="3. 烧录 SX.bin", width=14,
                                   command=self._flash, bg="#FF9800", fg="white")
        self.btn_flash.pack(side="left", padx=2)

        self.btn_all = tk.Button(btn_frm, text="一键: 编译+烧录", width=16,
                                 command=self._all, bg="#f44336", fg="white")
        self.btn_all.pack(side="left", padx=2)

        # ---- 串口 ----
        uart_frm = tk.Frame(root)
        uart_frm.pack(fill="x", padx=8, pady=2)
        tk.Label(uart_frm, text="串口:").pack(side="left")
        self.com_var = tk.StringVar(value="COM46")
        tk.Entry(uart_frm, textvariable=self.com_var, width=8).pack(side="left", padx=4)
        tk.Button(uart_frm, text="OTA升级", width=10, command=self._ota,
                  bg="#9C27B0", fg="white").pack(side="left", padx=4)

        # ---- 输出窗口 ----
        tk.Label(root, text="输出:").pack(anchor="w", padx=8)
        self.output = scrolledtext.ScrolledText(root, height=16, font=("Consolas", 9))
        self.output.pack(fill="both", expand=True, padx=8, pady=4)

    def _browse(self, var, pattern):
        path = filedialog.askopenfilename(filetypes=[(pattern, pattern)])
        if path: var.set(path)

    def _log(self, text):
        self.output.insert("end", text + "\n")
        self.output.see("end")
        self.root.update()

    def _run(self, cmd, cwd=None):
        self._log("> " + (" ".join(cmd) if isinstance(cmd, list) else cmd))
        try:
            p = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 text=True, encoding="utf-8", errors="replace", shell=True)
            for line in p.stdout:
                self._log(line.rstrip())
            p.wait()
            ok = p.returncode == 0
            self._log("=== %s ===\n" % ("OK" if ok else "FAILED (code=%d)" % p.returncode))
            return ok
        except Exception as e:
            self._log("ERROR: " + str(e))
            return False

    def _thread(self, fn):
        self.output.delete("1.0", "end")
        for b in [self.btn_build, self.btn_app, self.btn_flash, self.btn_all]:
            b.config(state="disabled")
        threading.Thread(target=lambda: [fn(), self._enable()], daemon=True).start()

    def _enable(self):
        for b in [self.btn_build, self.btn_app, self.btn_flash, self.btn_all]:
            b.config(state="normal")

    def _build_bl(self):
        self._thread(self._do_build_bl)

    def _do_build_bl(self):
        bl_src = self.bl_var.get()
        app_bin = self.app_var.get()
        sx_bin = self.sx_var.get()

        if not os.path.exists(bl_src):
            self._log("ERROR: BL 源文件不存在: " + bl_src); return
        if not os.path.exists(app_bin):
            self._log("WARNING: APP.bin 不存在, 请先编译 APP"); return

        # Run build.ps1 with parameters
        ps1 = os.path.join(BL_DIR, "build.ps1")
        cmd = f'powershell -ExecutionPolicy Bypass -File "{ps1}" -AppBin "{app_bin}"'
        if self._run(cmd, cwd=BL_DIR):
            # Check output
            sx_out = os.path.join(BL_DIR, "bootloader_pad.bin")
            sx_out2 = os.path.join(BL_DIR, "..", "output", "bin", "AMKN8639_SX.bin")
            if os.path.exists(sx_out2):
                size = os.path.getsize(sx_out2)
                self._log(f"SX.bin: {size} bytes ({size/1024:.1f} KB)")

    def _build_app(self):
        self._thread(self._do_build_app)

    def _do_build_app(self):
        if not os.path.exists(KEIL_UV4):
            self._log("ERROR: Keil 未找到: " + KEIL_UV4); return
        cmd = f'"{KEIL_UV4}" -j0 -b "{KEIL_PROJ}"'
        self._run(cmd)

    def _flash(self):
        self._thread(self._do_flash)

    def _do_flash(self):
        sx = self.sx_var.get()
        if not os.path.exists(sx):
            self._log("ERROR: SX.bin 不存在, 请先编译"); return
        if not os.path.exists(STM32_PROG):
            self._log("ERROR: STM32_Programmer 未找到"); return
        cmd = f'"{STM32_PROG}" -c port=SWD freq=4000 -e all --skipErase -w "{sx}" 0x08000000 -rst'
        self._run(cmd)

    def _ota(self):
        self._thread(self._do_ota)

    def _do_ota(self):
        app = self.app_var.get()
        if not os.path.exists(app):
            self._log("ERROR: APP.bin 不存在"); return
        ota_py = os.path.join(BASE_DIR, "scripts", "ota_host.py")
        com = self.com_var.get()
        cmd = f'python "{ota_py}" {com} auto "{app}"'
        self._run(cmd)

    def _all(self):
        self._thread(self._do_all)

    def _do_all(self):
        self._log("===== 一键编译 + 烧录 =====\n")
        if not self._do_build_bl():
            self._log("编译失败，停止")
            return
        self._do_flash()


if __name__ == "__main__":
    root = tk.Tk()
    app = BuildTool(root)
    root.mainloop()
