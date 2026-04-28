#!/usr/bin/env python3
import subprocess

syscalls_to_test = [
    ("SYS_WRITE", 1, 6),     # length output check
    ("SYS_MMAP", 9, -38),    # Ensure ENOSYS on phase 2
    ("UNSUPPORTED", 999, -38) # Complete strict fallback
]

def run_tests():
    print("Running Baseline Compliance Diff...")
    output = subprocess.check_output(["./mnmkernel_sim"], text=True)
    
    passed = 0
    for name, nr, expected in syscalls_to_test:
        if f"Returns: {expected}" in output:
            print(f"[PASS] {name} matched expected {expected}")
            passed += 1
        else:
            print(f"[FAIL] {name} did NOT match {expected}")
            
    print(f"\nResult: {passed}/{len(syscalls_to_test)} tests passed.")

if __name__ == "__main__":
    run_tests()
