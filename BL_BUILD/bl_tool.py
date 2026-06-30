#!/usr/bin/env python3
"""AMKN8639 BL Build Tool"""
import tkinter as tk
from tkinter import filedialog, scrolledtext
import subprocess, os, sys, io, threading

if sys.platform == "win32":
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
APP_DEFAULT = os.path.join(BASE_DIR, "..", "output", "bin", "AMKN8639_APP.bin")
SX_DEFAULT = os.path.join(BASE_DIR, "output", "AMKN8639_SX.bin")
KEIL_UV4 = r"D:\keil5\UV4\UV4.exe"
KEIL_PROJ = os.path.join(BASE_DIR, "..", "AMKN8639.uvprojx")
STM32_PROG = r"D:\stmp\bin\STM32_Programmer_CLI.exe"

class BuildTool:
    def __init__(self, root):
        self.root = root
        root.title("AMKN8639 BL Build Tool")
        root.geometry("680x520")
        root.resizable(True, True)

        frm = tk.LabelFrame(root, text="路径设置", padx=8, pady=4)
        frm.pack(fill="x", padx=8, pady=4)

        tk.Label(frm, text="BL 源文件:").grid(row=0, column=0, sticky="w")
        self.bl_var = tk.StringVar(value=os.path.join(BASE_DIR, "src", "bootloader.c"))
        tk.Entry(frm, textvariable=self.bl_var, width=60).grid(row=0, column=1, padx=4)
        tk.Button(frm, text="...", command=lambda: self._browse(self.bl_var, "*.c")).grid(row=0, column=2)

        tk.Label(frm, text="BL 固件:").grid(row=1, column=0, sticky="w", pady=2)
        self.blbin_var = tk.StringVar(value=os.path.join(BASE_DIR, "output", "bootloader.bin"))
        tk.Entry(frm, textvariable=self.blbin_var, width=60).grid(row=1, column=1, padx=4)
        tk.Button(frm, text="...", command=lambda: self._browse(self.blbin_var, "*.bin")).grid(row=1, column=2)

        tk.Label(frm, text="APP 固件:").grid(row=2, column=0, sticky="w", pady=2)
        self.app_var = tk.StringVar(value=APP_DEFAULT)
        tk.Entry(frm, textvariable=self.app_var, width=60).grid(row=2, column=1, padx=4)
        tk.Button(frm, text="...", command=lambda: self._browse(self.app_var, "*.bin")).grid(row=2, column=2)

        tk.Label(frm, text="输出 SX.bin:").grid(row=3, column=0, sticky="w", pady=2)
        self.sx_var = tk.StringVar(value=SX_DEFAULT)
        tk.Entry(frm, textvariable=self.sx_var, width=60).grid(row=3, column=1, padx=4); tk.Button(frm, text='...', command=lambda: self._browse(self.sx_var, '*.bin')).grid(row=3, column=2)

        btn_frm = tk.Frame(root)
        btn_frm.pack(fill="x", padx=8, pady=4)
        self.btn_blonly = tk.Button(btn_frm, text="1a. 仅编译 BL", width=12, command=self._build_bl_only, bg="#8BC34A", fg="white")
        self.btn_blonly.pack(side="left", padx=2)
        self.btn_build = tk.Button(btn_frm, text="1b. 编译 BL + 打包", width=16, command=self._build_bl, bg="#4CAF50", fg="white")
        self.btn_build.pack(side="left", padx=2)
        self.btn_app = tk.Button(btn_frm, text="2. 编译 APP", width=14, command=self._build_app, bg="#2196F3", fg="white")
        self.btn_app.pack(side="left", padx=2)
        self.btn_flash = tk.Button(btn_frm, text="3. 烧录 SX.bin", width=14, command=self._flash, bg="#FF9800", fg="white")
        self.btn_flash.pack(side="left", padx=2)
        self.btn_all = tk.Button(btn_frm, text="一键: 编译+烧录", width=16, command=self._all, bg="#f44336", fg="white")
        self.btn_all.pack(side="left", padx=2)

        uart_frm = tk.Frame(root)
        uart_frm.pack(fill="x", padx=8, pady=2)
        tk.Label(uart_frm, text="串口:").pack(side="left")
        self.com_var = tk.StringVar(value="COM46")
        tk.Entry(uart_frm, textvariable=self.com_var, width=8).pack(side="left", padx=4)
        tk.Button(uart_frm, text="OTA升级APP", width=12, command=self._ota, bg="#9C27B0", fg="white").pack(side="left", padx=2)
        tk.Button(uart_frm, text="OTA升级BL", width=12, command=self._ota_bl, bg="#E91E63", fg="white").pack(side="left", padx=2)

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

    def _run(self, cmd):
        self._log("> " + cmd)
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 text=True, encoding="utf-8", errors="replace", shell=True)
            for line in p.stdout: self._log(line.rstrip())
            p.wait()
            ok = p.returncode == 0
            self._log("=== %s ===\n" % ("OK" if ok else "FAILED"))
            return ok
        except Exception as e:
            self._log("ERROR: " + str(e))
            return False

    def _thread(self, fn):
        self.output.delete("1.0", "end")
        for b in [self.btn_blonly, self.btn_blonly, self.btn_build, self.btn_app, self.btn_flash, self.btn_all]:
            b.config(state="disabled")
        threading.Thread(target=lambda: [fn(), self._enable()], daemon=True).start()

    def _enable(self):
        for b in [self.btn_blonly, self.btn_blonly, self.btn_build, self.btn_app, self.btn_flash, self.btn_all]:
            b.config(state="normal")

    def _build_bl(self): self._thread(self._do_build_bl)
    def _build_bl_only(self): self._thread(self._do_build_bl_only)
    def _do_build_bl_only(self):
        ps1 = os.path.join(BASE_DIR, "scripts", "build.ps1")
        self._run(f"powershell -ExecutionPolicy Bypass -File \"{ps1}\" -BlOnly")
    def _do_build_bl(self):
        ps1 = os.path.join(BASE_DIR, "scripts", "build.ps1")
        app = self.app_var.get()
        if not os.path.exists(app): self._log("WARNING: APP.bin 不存在"); return
        self._run(f'powershell -ExecutionPolicy Bypass -File "{ps1}" -AppBin "{app}" -SxOut "{self.sx_var.get()}"')

    def _build_app(self): self._thread(self._do_build_app)
    def _do_build_app(self):
        if not os.path.exists(KEIL_UV4): self._log("ERROR: Keil not found"); return
        self._run(f'"{KEIL_UV4}" -j0 -b "{KEIL_PROJ}"')

    def _flash(self): self._thread(self._do_flash)
    def _do_flash(self):
        sx = self.sx_var.get()
        if not os.path.exists(sx): self._log("ERROR: SX.bin 不存在"); return
        self._run(f'"{STM32_PROG}" -c port=SWD freq=4000 -e all --skipErase -w "{sx}" 0x08000000 -rst')

    def _ota(self): self._thread(self._do_ota)
    def _ota_bl(self): self._thread(self._do_ota_bl)
    def _do_ota_bl(self):
        bl_bin = self.blbin_var.get()
        if not os.path.exists(bl_bin):
            self._log("ERROR: 请先 1a. 仅编译 BL"); return
        ota_py = os.path.join(BASE_DIR, "scripts", "ota_host.py")
        self._run(f"python \"{ota_py}\" {self.com_var.get()} flash-bl \"{bl_bin}\"")
    def _do_ota(self):
        app = self.app_var.get()
        if not os.path.exists(app): self._log("ERROR: APP.bin 不存在"); return
        ota_py = os.path.join(BASE_DIR, "scripts", "ota_host.py")
        self._run(f'python "{ota_py}" {self.com_var.get()} auto "{app}"')

    def _all(self): self._thread(self._do_all)
    def _do_all(self):
        if not self._do_build_bl(): return
        self._do_flash()

if __name__ == "__main__":
    root = tk.Tk()
    BuildTool(root)
    root.mainloop()



