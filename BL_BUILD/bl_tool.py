#!/usr/bin/env python3
r"""AMKN8639 BL Build Tool V4.0 — bind to current ota_host.py v4.0 (2026-07)

Aligned with the actual production toolchain:
  * scripts\ota_host.py     - CLI OTA host (v4.0, read_dtcm fixed)
  * scripts\build.ps1       - BL build/script with -BlOnly / -AppBin / -SxOut
  * src\bootloader.c        - BL source (Thumb-bit fix; DTCM save points)
  * D:\keil5\UV4\UV4.exe    - ARMCC v5 + Keil UV4 (compile APP)
  * D:\stmp\bin\STM32_Programmer_CLI.exe - SWD flash + memory read

Usage:
    Double-click BL工具.bat
    or:   python bl_tool.py
"""
import os, sys, io, subprocess, threading
import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext, messagebox

if sys.platform == "win32":
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

try:
    import serial.tools.list_ports as list_ports
except Exception:
    list_ports = None  # scan button will be disabled

BASE_DIR   = os.path.dirname(os.path.abspath(__file__))
OTA_HOST   = os.path.join(BASE_DIR, "scripts", "ota_host.py")
BUILD_PS1  = os.path.join(BASE_DIR, "scripts", "build.ps1")

# default APP & SX paths, must agree with build.ps1 defaults
APP_DEFAULT = os.path.normpath(os.path.join(BASE_DIR, "..", "output", "bin", "AMKN8639_APP.bin"))
SX_DEFAULT  = os.path.normpath(os.path.join(BASE_DIR, "output", "AMKN8639_SX.bin"))
BLBIN_DEF   = os.path.normpath(os.path.join(BASE_DIR, "output", "bootloader.bin"))

# external tool paths (edit if your host differs)
KEIL_UV4    = r"D:\keil5\UV4\UV4.exe"
KEIL_PROJ   = os.path.normpath(os.path.join(BASE_DIR, "..", "AMKN8639.uvprojx"))
STM32_PROG  = r"D:\stmp\bin\STM32_Programmer_CLI.exe"
OTA_DEFAULT_PORT = "COM46"


def list_serial_ports():
    if list_ports is None:
        return []
    return sorted([p.device for p in list_ports.comports()])


class BuildTool:
    def __init__(self, root):
        self.root = root
        root.title("AMKN8639 BL Build Tool V4.0  (ota_host v4.0)")
        root.geometry("760x600")
        root.resizable(True, True)

        # ---------- path block ----------
        frm = tk.LabelFrame(root, text="路径 (defaults match scripts\\build.ps1)", padx=8, pady=4)
        frm.pack(fill="x", padx=8, pady=4)

        self.bl_var    = tk.StringVar(value=os.path.join(BASE_DIR, "src", "bootloader.c"))
        self.blbin_var = tk.StringVar(value=BLBIN_DEF)
        self.app_var   = tk.StringVar(value=APP_DEFAULT)
        self.sx_var    = tk.StringVar(value=SX_DEFAULT)

        rows = [
            ("BL 源文件:",   self.bl_var,    "*.c"),
            ("BL 固件:",     self.blbin_var, "*.bin"),
            ("APP 固件:",    self.app_var,   "*.bin"),
            ("输出 SX.bin:", self.sx_var,    "*.bin"),
        ]
        for i, (label, var, pat) in enumerate(rows):
            tk.Label(frm, text=label).grid(row=i, column=0, sticky="w", pady=2)
            tk.Entry(frm, textvariable=var, width=70).grid(row=i, column=1, padx=4, sticky="we")
            tk.Button(frm, text="…", width=3,
                      command=lambda v=var, p=pat: self._browse(v, p)).grid(row=i, column=2)
        frm.columnconfigure(1, weight=1)

        # ---------- action buttons ----------
        btn_frm = tk.LabelFrame(root, text="编译 / 烧录", padx=8, pady=4)
        btn_frm.pack(fill="x", padx=8, pady=4)

        self.btn_blonly = tk.Button(btn_frm, text="1a. 仅编译 BL", width=14, command=self._build_bl_only, bg="#8BC34A", fg="white")
        self.btn_blonly.grid(row=0, column=0, padx=2, pady=2)
        self.btn_build  = tk.Button(btn_frm, text="1b. 编译 BL + 打包", width=18, command=self._build_bl, bg="#4CAF50", fg="white")
        self.btn_build.grid(row=0, column=1, padx=2, pady=2)
        self.btn_app    = tk.Button(btn_frm, text="2. 编译 APP (Keil)", width=18, command=self._build_app, bg="#2196F3", fg="white")
        self.btn_app.grid(row=0, column=2, padx=2, pady=2)
        self.btn_flash  = tk.Button(btn_frm, text="3. 烧录 SX.bin (SWD)", width=20, command=self._flash, bg="#FF9800", fg="white")
        self.btn_flash.grid(row=0, column=3, padx=2, pady=2)
        self.btn_all    = tk.Button(btn_frm, text="一键: 编译+烧录", width=18, command=self._all, bg="#f44336", fg="white")
        self.btn_all.grid(row=0, column=4, padx=2, pady=2)

        # ---------- OTA / diag block ----------
        ota_frm = tk.LabelFrame(root, text="串口 OTA / 诊断 (通过 ota_host.py v4.0)", padx=8, pady=4)
        ota_frm.pack(fill="x", padx=8, pady=4)

        tk.Label(ota_frm, text="串口:").grid(row=0, column=0, sticky="w")
        ports = list_serial_ports()
        if OTA_DEFAULT_PORT not in ports:
            ports = [OTA_DEFAULT_PORT] + ports
        self.com_var = tk.StringVar(value=OTA_DEFAULT_PORT)
        self.com_box = ttk.Combobox(ota_frm, textvariable=self.com_var, width=10, values=ports)
        self.com_box.grid(row=0, column=1, sticky="w")
        tk.Button(ota_frm, text="扫描端口", width=8, command=self._scan_ports).grid(row=0, column=2, padx=4)

        self.btn_ota    = tk.Button(ota_frm, text="OTA 升级 APP", width=14, command=self._ota,      bg="#9C27B0", fg="white")
        self.btn_otabl  = tk.Button(ota_frm, text="OTA 升级 BL",  width=14, command=self._ota_bl,   bg="#E91E63", fg="white")
        self.btn_info   = tk.Button(ota_frm, text="查询 BL",      width=10, command=self._ota_info, bg="#00BCD4", fg="white")
        self.btn_test   = tk.Button(ota_frm, text="端到端测试",   width=12, command=self._ota_test, bg="#3F51B5", fg="white")
        self.btn_diag   = tk.Button(ota_frm, text="诊断 DTCM",    width=12, command=self._ota_diag, bg="#009688", fg="white")
        self.btn_reset  = tk.Button(ota_frm, text="SWD 复位",     width=10, command=self._ota_reset, bg="#795548", fg="white")

        self.btn_ota.grid(  row=1, column=0, padx=2, pady=4, sticky="we")
        self.btn_otabl.grid(row=1, column=1, padx=2, pady=4, sticky="we")
        self.btn_info.grid( row=1, column=2, padx=2, pady=4, sticky="we")
        self.btn_test.grid( row=1, column=3, padx=2, pady=4, sticky="we")
        self.btn_diag.grid( row=1, column=4, padx=2, pady=4, sticky="we")
        self.btn_reset.grid(row=1, column=5, padx=2, pady=4, sticky="we")

        for c in range(6):
            ota_frm.columnconfigure(c, weight=1)

        self.all_buttons = [
            self.btn_blonly, self.btn_build, self.btn_app,
            self.btn_flash,  self.btn_all,
            self.btn_ota, self.btn_otabl, self.btn_info,
            self.btn_test,   self.btn_diag, self.btn_reset,
        ]

        # ---------- output ----------
        tk.Label(root, text="输出:").pack(anchor="w", padx=8)
        self.output = scrolledtext.ScrolledText(root, height=18, font=("Consolas", 9))
        self.output.pack(fill="both", expand=True, padx=8, pady=4)

        # status bar
        self.status_var = tk.StringVar(value="Ready. Default port: " + OTA_DEFAULT_PORT)
        tk.Label(root, textvariable=self.status_var, anchor="w", relief="sunken").pack(fill="x", side="bottom")

    # ---- UI helpers ----
    def _browse(self, var, pattern):
        path = filedialog.askopenfilename(filetypes=[(pattern, pattern)])
        if path:
            var.set(path)

    def _scan_ports(self):
        ports = list_serial_ports()
        if not ports:
            self._log("[scan] no COM ports detected")
            messagebox.showwarning("扫描", "未发现可用串口")
            return
        self.com_box["values"] = ports
        if self.com_var.get() not in ports:
            self.com_var.set(ports[0])
        self._log("[scan] COM ports: %s" % ports)

    def _log(self, text):
        self.output.insert("end", text + "\n")
        self.output.see("end")
        self.root.update_idletasks()

    def _set_status(self, text):
        self.status_var.set(text)
        self.root.update_idletasks()

    def _run_blocking(self, cmd):
        self._log("> " + cmd)
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 text=True, encoding="utf-8", errors="replace", shell=True)
            for line in p.stdout:
                self._log(line.rstrip())
            p.wait()
            ok = p.returncode == 0
            self._log("=== %s ===" % ("OK" if ok else "FAILED (rc=%d)" % p.returncode))
            return ok
        except Exception as e:
            self._log("ERROR: " + str(e))
            return False

    def _run_in_thread(self, fn, label):
        for b in self.all_buttons:
            b.config(state="disabled")
        self._set_status("Running: " + label)
        def task():
            try:
                fn()
            finally:
                self._set_status("Idle")
                for b in self.all_buttons:
                    b.config(state="normal")
        threading.Thread(target=task, daemon=True).start()

    # ---- path wrappers ----
    def _port(self):
        return self.com_var.get().strip() or OTA_DEFAULT_PORT

    # ---- build / flash actions ----
    def _build_bl_only(self):
        def do():
            ps1 = BUILD_PS1
            self._run_blocking('powershell -ExecutionPolicy Bypass -File "%s" -BlOnly' % ps1)
        self._run_in_thread(do, "仅编译 BL")

    def _build_bl(self):
        def do():
            app = self.app_var.get()
            if not os.path.exists(app):
                self._log("WARNING: APP.bin 不存在，仍尝试打包 (%s)" % app)
            self._run_blocking('powershell -ExecutionPolicy Bypass -File "%s" -AppBin "%s" -SxOut "%s"'
                              % (BUILD_PS1, app, self.sx_var.get()))
        self._run_in_thread(do, "编译 BL + 打包")

    def _build_app(self):
        def do():
            if not os.path.exists(KEIL_UV4):
                self._log("ERROR: Keil UV4 not found: " + KEIL_UV4); return
            self._run_blocking('"%s" -j0 -b "%s"' % (KEIL_UV4, KEIL_PROJ))
        self._run_in_thread(do, "编译 APP")

    def _flash(self):
        def do():
            sx = self.sx_var.get()
            if not os.path.exists(sx):
                self._log("ERROR: SX.bin 不存在: " + sx); return
            self._run_blocking('"%s" -c port=SWD freq=4000 -e all --skipErase -w "%s" 0x08000000 -rst'
                              % (STM32_PROG, sx))
        self._run_in_thread(do, "SWD 烧录 SX.bin")

    def _all(self):
        def do():
            self._run_blocking('powershell -ExecutionPolicy Bypass -File "%s" -AppBin "%s" -SxOut "%s"'
                              % (BUILD_PS1, self.app_var.get(), self.sx_var.get()))
            sx = self.sx_var.get()
            if os.path.exists(sx):
                self._run_blocking('"%s" -c port=SWD freq=4000 -e all --skipErase -w "%s" 0x08000000 -rst'
                                  % (STM32_PROG, sx))
        self._run_in_thread(do, "一键: 编译+烧录")

    # ---- OTA actions ----
    def _ota_cmd(self, subcmd, *args, mode=""):
        port = self._port()
        cmd = ['python', OTA_HOST, port, subcmd] + list(args)
        full = subprocess.list2cmdline(cmd)
        def do():
            self._run_blocking(full)
        label = "[%s] %s %s" % (port, subcmd, " ".join(args))
        self._run_in_thread(do, label)

    def _ota(self):       self._ota_cmd("auto", self.app_var.get())
    def _ota_bl(self):    self._ota_cmd("flash-bl", self.blbin_var.get())
    def _ota_info(self):  self._ota_cmd("info")
    def _ota_test(self):  self._ota_cmd("test")
    def _ota_diag(self):  self._ota_cmd("diag")
    def _ota_reset(self): self._ota_cmd("reset")


if __name__ == "__main__":
    root = tk.Tk()
    BuildTool(root)
    root.mainloop()
