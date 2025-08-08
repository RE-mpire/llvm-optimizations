# LLVM 15 Optimizations

My reimplementation of common compiler optimizations.

## Table of Contents

- [Overview](#overview)
- [Implemented Optimizations](#implemented-optimizations)
- [References](#references)

## Overview

The goal of this repository is to explore, implement, and document optimization techniques for LLVM 15. These optimizations are not designed for production use but are intended primarily for educational purposes.
## Implemented Optimizations

- **Constant Folding**: Evaluate constant expressions at compile time.
- **Common Subexpression Elimination**: Avoid redundant calculations by reusing results.
- **Loop Unrolling**: Increase performance by reducing the overhead of loop control or small loops.
- **Strength Reduction**: Replace expensive operations with cheaper ones. Promoted Alloca instructions to registers to reduce memory access overhead.

## References

- [Compiler Optimizations](https://en.wikipedia.org/wiki/Compiler_optimization)
- [LLVM Optimizations](https://llvm.org/docs/Passes.html)

---
*This repository is dedicated solely to llvm 15 compiler*