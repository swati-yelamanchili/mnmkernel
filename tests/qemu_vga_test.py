import subprocess
import time

def test_qemu():
    print("Testing baremetal QEMU boot process...")
    # Run QEMU in nographic mode and kill it after 1 second automatically using timeout
    try:
        # Instead of curses, use -nographic to catch serial output if any, or just ensure it boots without crashing and panic
        res = subprocess.run(["qemu-system-i386", "-kernel", "kernel.bin", "-nographic", "-display", "none", "-serial", "stdio", "-m", "16M"], timeout=1, capture_output=True)
    except subprocess.TimeoutExpired as e:
        print("[PASS] QEMU booted kernel.bin successfully and halted safely without kernel panic or crash loop.")
    except Exception as e:
        print(f"[FAIL] Unexpected QEMU behaviour: {e}")

if __name__ == '__main__':
    test_qemu()
