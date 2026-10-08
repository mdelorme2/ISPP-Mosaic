# A regular perspective on solving the irregular strip packing problem with the dotted-board model: a mosaic approach and a divide-and-conquer strategy

This repository contains the C++ implementations of all algorithms presented in the paper:
> **A regular perspective on solving the irregular strip packing problem with the dotted-board model: a mosaic approach and a divide-and-conquer strategy**  
> *[Maxence Delorme](https://www.tilburguniversity.edu/staff/m-delorme) and [José Fernando Oliveira](https://sigarra.up.pt/feup/en/func_geral.formview?p_codigo=209980)*

## Dependencies

Our C++ algorithms rely on the following:
- **[Gurobi Optimizer](https://www.gurobi.com/):** Commercial ILP solver (requires a valid license).
- **[Clipper2](https://github.com/AngusJohnson/Clipper2):** Polygon clipping library used to detect pairwise incompatibilities and construct mosaics.
- **[ECC8](https://github.com/Pronte/ECC):** Software library used to compute a minimum edge clique cover.

---

## List of Approaches

This repository includes 12 approaches corresponding to the methods described in the paper:

| Folder | Method Description |
| :--- | :--- |
| `1_PAIRWISE` | The PAIRWISE approach |
| `2_CLIQUE_X1` | The CLIQUEx1 approach |
| `3_CLIQUE_X10` | The CLIQUEx10 approach |
| `4_MOSAICSC` | The MOSAIC approach |
| `5_MTSC` | The MT 1×1 approach with MT_SC |
| `6_MTOSC` | The MT 1×1 approach with MT^O_SC (mosaic size selectable via parameters) |
| `7_MTOSCF` | The MT 1×1 approach with MT^O_SC-F (mosaic size selectable via parameters) |
| `8_MTOSCF_NZ` | Best MT 1×1 approach & non-zero reduction strategy (12 configurations via parameters) |
| `9_MTOSCF_NZ_MV` | MT 1×1 OBJ1, OBJ2, and OBJ3 approaches (selectable via parameters) |
| `A_MTOSCF_NZ_DB` | The MT 1×1 OBJ4 approach |
| `B_MTOSCF_NZ_LINKF` | The MT 1×1 OBJ5 approach |
| `C_MTOSCF_NZ_LINKFANDINC` | The MT 1×1 OBJ6 approach |

---

## Folder Structure

Each approach folder shares a common code structure. For example, `1_PAIRWISE` contains:

| File | Description |
| :--- | :--- |
| `helper_functions_part1.cpp` | Input/Output helper functions |
| `helper_functions_part2.cpp` | Secondary functions for PAIRWISE and CLIQUE approaches |
| `helper_functions_part3.cpp` | Secondary functions for mosaic approaches |
| `helper_functions.h` | Header file for `helper_functions_part{1,2,3}.cpp` |
| `main.cpp` | Front-end code for the method *(varies by approach)* |
| `main.h` | Header file corresponding to `main.cpp` |
| `makefile` | Compilation script for Linux *(user must update library paths)* |
| `time.cpp` / `time.h` | Generic module for measuring CPU computation time |

> **Note:** All files except `main.cpp` are identical across all approach folders.

Additionally, the root folder includes `Draw.py`, a Python utility script to convert output solution files into visual figures.

---

## Running an Approach

1. Compile the project using the provided `makefile` inside the desired approach folder.
2. Execute the compiled executable:

```
./PROGRAM "./PATH_INSTANCE" "NAME_INSTANCE" "./PATH_AND_NAME_OUTPUT_GENERAL" "./PATH_AND_NAME_SOLUTION"
```

**Parameters:**
- `PROGRAM`: Name of the compiled binary.
- `./PATH_INSTANCE`: Relative path to the folder containing the instance file.
- `NAME_INSTANCE`: Name of the target instance file.
- `./PATH_AND_NAME_OUTPUT_GENERAL`: File path to write performance metrics (e.g., optimality status, CPU time, number of variables).
- `./PATH_AND_NAME_OUTPUT_SOLUTION`: File path to save the solution file (compatible with `Draw.py`).

An example execution script (`script_11.sh`) is provided in the repository.

---

## Contact & Feedback

For questions, bug reports, or suggestions, please contact:  
**Maxence Delorme** — `m.delorme[at]tilburguniversity[dot]edu` *(Subject line: **ISPP**)*
