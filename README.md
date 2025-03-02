# Supplementary Material: Compilation and Execution Guide  

The programs provided in the `supplementary_material` directory are designed to compile and execute on any **Unix** or **Unix-like** operating system.
They have been tested and successfully compiled using the **g++** compiler on **Linux**.  

## Compilation Instructions  
To build the binaries, open a terminal in the `supplementary_material` directory and run the following commands:  

```bash
make
cd bin
```

### Output and Binary Naming Convention
Upon successful compilation, the executable binaries will be generated inside the bin/ directory. The naming convention of these binaries reflects the number of tweak blocks used in the benchmarking process.
For instance, a binary named benchmark-i corresponds to benchmarking results for all primitives with $i \times n$-bit tweaks, where $n$ represents the state size.
