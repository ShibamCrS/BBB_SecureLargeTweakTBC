# Supplementary Material: Compilation and Execution Guide

The programs provided in the `supplementary_material` directory are designed to compile and execute on any **Unix** or **Unix-like** operating system.  
They have been tested and successfully compiled using the **g++** compiler on **Linux**.

---
## Requirements

### Hardware
- x86_64 CPU with support for the PCLMULQDQ instruction set (CLMUL)
- AES-NI support

You can check CLMUL support with:
```bash
grep pclmul /proc/cpuinfo
```

## 1. Compilation and Execution

### For Users with Root Privileges

To build the binaries and run all benchmarks, open a terminal in the `supplementary_material` directory and run:

```bash
./benchmarks
```

### Output and Binary Naming Convention

Upon successful compilation:
- Executable binaries are generated inside the `bin/` directory.
- The binary names indicate the number of tweak blocks used in the benchmarking.
- For example, `benchmark-i` corresponds to benchmarks using tweaks of size $ i \times n $ bits, where $ n $ is the state size.
- The default run (`./benchmarks`) executes `benchmark-1`, `benchmark-2`, `benchmark-3`, and `benchmark-4` one after another.

It is also possible to run benchmarks for individual tweak sizes.
To run a specific benchmark (e.g., `benchmark-3`), use:

```bash
make
./benchmarks ./bin/benchmark-3
```

## Benchmarking Tools and Modes

By default, the benchmarking process utilizes the **rdpmc** (Read Performance Monitoring Counter) instruction to measure cycles per byte.
This method provides the most accurate results but requires setting up performance counters, which typically necessitates **root privileges**.

---

### Running Benchmarks Without Root Privileges

For users without root access, the **rdtsc** (Read Time-Stamp Counter) instruction can be used as an alternative.
However, `rdtsc` is platform-dependent: it is accurate on Intel CPUs but may exhibit synchronization issues on AMD CPUs.
To switch to `rdtsc` mode, run the benchmarks as follows:

```bash
ROOT=0 BENCHTOOL=USE_RDTSC ./benchmarks
```
or

```bash
make BENCHTOOL=USE_RDTSC
ROOT=0 BENCHTOOL=USE_RDTSC ./benchmarks ./bin/benchmark-3
```
