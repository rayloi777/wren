#!/usr/bin/env python3
"""Simple benchmark comparison script - Wren vs C."""

import subprocess
import sys

def run_cmd(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd="/Users/user/Documents/GitHub/wren")
    return result.stdout, result.returncode

benchmarks = [
    ("loop", "./bin/wren_test test/benchmark/loop.wren", "test/benchmark/loop_c"),
    ("list_construct", "./bin/wren_test test/benchmark/list_construct.wren", "test/benchmark/list_construct_c"),
    ("map_construct", "./bin/wren_test test/benchmark/map_construct.wren", "test/benchmark/map_construct_c"),
    ("string_concat", "./bin/wren_test test/benchmark/string_concat.wren", "test/benchmark/string_concat_c"),
    ("n_body", "./bin/wren_test test/benchmark/n_body.wren", "test/benchmark/n_body_c 500000"),
    ("fannkuch", "./bin/wren_test test/benchmark/fannkuch.wren", "test/benchmark/fannkuch_c 8"),
    ("spectral_norm", "./bin/wren_test test/benchmark/spectral_norm.wren", "test/benchmark/spectral_norm_c 100"),
]

print("="*70)
print("Wren vs C Benchmark Results")
print("="*70)
print(f"{'Benchmark':<20} {'Wren (s)':<15} {'C -O0 (s)':<15} {'Ratio':<10}")
print("-"*70)

for name, wren_cmd, c_cmd in benchmarks:
    wren_out, wren_ret = run_cmd(wren_cmd)
    wren_lines = wren_out.strip().split('\n')
    wren_time = float(wren_lines[-1].split(': ')[1])
    
    c_out, c_ret = run_cmd(c_cmd)
    c_lines = c_out.strip().split('\n')
    c_time = float(c_lines[-1].split(': ')[1])
    
    ratio = wren_time / c_time if c_time > 0 else 0
    
    print(f"{name:<20} {wren_time:<15.4f} {c_time:<15.4f} {ratio:<10.2f}x")

print("="*70)
print("\nNote: C benchmarks compiled with -O0 (no optimization)")
