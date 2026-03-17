#!/usr/bin/env python3
"""Simple Vec4 benchmark comparison script."""

import subprocess
import re

def run_cmd(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout

def parse_wren_output(output):
    results = {}
    for line in output.strip().split('\n'):
        match = re.match(r'(\w+): ([\d.]+)', line)
        if match:
            results[match.group(1)] = float(match.group(2))
    return results

def parse_c_output(output):
    results = {}
    for line in output.strip().split('\n'):
        match = re.match(r'(\w+): ([\d.]+)', line)
        if match:
            results[match.group(1)] = float(match.group(2))
    return results

print("Running Wren benchmark...")
wren_output = run_cmd('./bin/wren_test test/ray/vec4_bench.wren')
wren_results = parse_wren_output(wren_output)

print("Running C benchmark (compiled with -O0)...")
c_output = run_cmd('./bin/vec4_bench_c')
c_results = parse_c_output(c_output)

print("\n" + "="*60)
print("Vec4 Benchmark Results (10,000,000 iterations)")
print("="*60)
print(f"{'Operation':<15} {'Wren (s)':<12} {'C -O0 (s)':<12} {'Ratio':<12}")
print("-"*60)

for op in ['add', 'sub', 'mul', 'div', 'dot', 'length', 'normalize']:
    wren_time = wren_results.get(op, 0)
    c_time = c_results.get(op, 0)
    
    ratio = wren_time / c_time if c_time > 0 else 0
    
    print(f"{op:<15} {wren_time:<12.4f} {c_time:<12.4f} {ratio:<12.2f}x")

print("="*60)
print("\nNote: C compiled with -O0 (no optimization)")
print("      Wren is compared against optimized C code")
