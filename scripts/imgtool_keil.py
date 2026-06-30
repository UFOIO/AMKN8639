#!/usr/bin/env python3
"""
imgtool_keil.py — AMKN8639 firmware signing tool

Wraps MCUBoot's imgtool to sign Keil-generated .bin files.
Produces MCUBoot-compatible signed images for bootloader_v2.

Prerequisites:
    pip install imgtool

Usage:
    # Generate key pair (once)
    python imgtool_keil.py keygen -o amkn8639_root.pem

    # Sign firmware
    python imgtool_keil.py sign \
        --key amkn8639_root.pem \
        --bin AMKN8639_APP.bin \
        --version 1.5.0 \
        --output AMKN8639_APP_SIGNED.bin

    # Merge BL + signed APP
    python imgtool_keil.py merge \
        --bl bootloader_v2_pad.bin \
        --app AMKN8639_APP_SIGNED.bin \
        --output AMKN8639_SX.bin
"""
import argparse
import subprocess
import sys
import os
import struct
from pathlib import Path

SLOT_A_BASE = 0x08020000
SLOT_SIZE = 0x00180000  # 1536KB
HEADER_SIZE = 0x200     # 512 bytes
ALIGN = 8

def run_imboot(*args):
    """Run imgtool with given arguments."""
    cmd = [sys.executable, "-m", "imgtool.main"] + list(args)
    print(f"  {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"ERROR: {result.stderr}")
        sys.exit(1)
    print(f"  OK: {result.stdout.strip()}")

def cmd_keygen(args):
    """Generate ECDSA P-256 key pair."""
    run_imboot("keygen",
               "-t", "ecdsa-p256",
               "-k", args.output)
    print(f"\nKey saved to: {args.output}")
    print(f"Extract public key: python {sys.argv[0]} getpub -k {args.output}")

def cmd_getpub(args):
    """Extract public key from private key."""
    run_imboot("getpub", "-k", args.key)
    print(f"\nEmbed the 'pub_key' and 'pub_key_len' values into crypto_port.h")

def cmd_sign(args):
    """Sign firmware binary."""
    if not os.path.exists(args.bin):
        print(f"ERROR: Input binary not found: {args.bin}")
        sys.exit(1)

    if not os.path.exists(args.key):
        print(f"ERROR: Key not found: {args.key}")
        sys.exit(1)

    run_imboot("sign",
               "--key", args.key,
               "--header-size", str(HEADER_SIZE),
               "--align", str(ALIGN),
               "--version", args.version,
               "--slot-size", str(SLOT_SIZE),
               "--load-addr", hex(SLOT_A_BASE),
               "--pad",
               args.bin,
               args.output)

    signed_size = os.path.getsize(args.output)
    orig_size = os.path.getsize(args.bin)
    overhead = signed_size - orig_size
    print(f"\nOriginal:  {orig_size:,} bytes")
    print(f"Signed:     {signed_size:,} bytes")
    print(f"Overhead:   {overhead:,} bytes (header + TLVs)")
    print(f"Slot usage: {signed_size * 100 / SLOT_SIZE:.1f}% of {SLOT_SIZE//1024}KB")

def cmd_merge(args):
    """Merge bootloader + signed APP into single SX.bin for flashing."""
    bl_path = args.bl
    app_path = args.app
    output = args.output

    if not os.path.exists(bl_path):
        print(f"ERROR: Bootloader not found: {bl_path}")
        sys.exit(1)
    if not os.path.exists(app_path):
        print(f"ERROR: APP not found: {app_path}")
        sys.exit(1)

    bl_size = os.path.getsize(bl_path)
    app_size = os.path.getsize(app_path)

    if bl_size != 128 * 1024:
        print(f"WARNING: Bootloader size is {bl_size} bytes (expected 131072)")

    with open(output, "wb") as out:
        with open(bl_path, "rb") as f:
            out.write(f.read())
        with open(app_path, "rb") as f:
            out.write(f.read())

    total = os.path.getsize(output)
    print(f"\nBootloader:  {bl_size:>10,} bytes")
    print(f"APP:          {app_size:>10,} bytes")
    print(f"───")
    print(f"SX.bin:       {total:>10,} bytes")
    print(f"\nFlash command:")
    print(f"  ST-LINK_CLI -P {output} 0x08000000")

def cmd_info(args):
    """Display info about signed firmware image."""
    if not os.path.exists(args.bin):
        print(f"ERROR: File not found: {args.bin}")
        sys.exit(1)

    run_imboot("verify", args.bin)

def cmd_verify(args):
    """Verify signed firmware image."""
    if not os.path.exists(args.bin):
        print(f"ERROR: File not found: {args.bin}")
        sys.exit(1)
    if not os.path.exists(args.key):
        print(f"ERROR: Key not found: {args.key}")
        sys.exit(1)

    run_imboot("verify", "-k", args.key, args.bin)
    print("\n✓ Signature VALID")

def main():
    parser = argparse.ArgumentParser(
        description="AMKN8639 Firmware Signing Tool (MCUBoot imgtool wrapper)"
    )
    sub = parser.add_subparsers(dest="command", help="Commands")

    # keygen
    p = sub.add_parser("keygen", help="Generate ECDSA P-256 key pair")
    p.add_argument("-o", "--output", default="amkn8639_root.pem", help="Key output path")

    # getpub
    p = sub.add_parser("getpub", help="Extract public key")
    p.add_argument("-k", "--key", default="amkn8639_root.pem", help="Private key")

    # sign
    p = sub.add_parser("sign", help="Sign firmware binary")
    p.add_argument("--key", default="amkn8639_root.pem", help="Private key")
    p.add_argument("--bin", required=True, help="Input .bin file")
    p.add_argument("--version", required=True, help="Version (e.g. 1.5.0)")
    p.add_argument("-o", "--output", default="AMKN8639_APP_SIGNED.bin", help="Output file")

    # merge
    p = sub.add_parser("merge", help="Merge BL + signed APP for flashing")
    p.add_argument("--bl", default="bootloader_v2_pad.bin", help="Bootloader binary (128KB padded)")
    p.add_argument("--app", default="AMKN8639_APP_SIGNED.bin", help="Signed APP binary")
    p.add_argument("-o", "--output", default="AMKN8639_SX.bin", help="Output SX.bin")

    # info
    p = sub.add_parser("info", help="Show signed image info")
    p.add_argument("--bin", required=True, help="Signed .bin file")

    # verify
    p = sub.add_parser("verify", help="Verify signature")
    p.add_argument("--key", default="amkn8639_root.pem", help="Private/public key")
    p.add_argument("--bin", required=True, help="Signed .bin file")

    args = parser.parse_args()

    cmds = {
        "keygen": cmd_keygen,
        "getpub": cmd_getpub,
        "sign": cmd_sign,
        "merge": cmd_merge,
        "info": cmd_info,
        "verify": cmd_verify,
    }

    if args.command in cmds:
        cmds[args.command](args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
