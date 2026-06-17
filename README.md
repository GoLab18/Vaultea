# Vaultea

**A performance-focused, encrypted, local-first storage engine and credential manager.**

Vaultea is a custom-built C++ database engine designed specifically for secure credential and identity storage. It uses a slotted-page architecture and B-Tree indexing, enhanced by heuristic-based compression, secured by modern cryptography.

Currently, it includes a simple graphical frontend built with Qt6, but the core engine (`src/core`, `src/storage`, etc.) is fully decoupled and can be integrated into TUIs, CLIs, or headless servers.

## Architecture Highlights

* **Custom Storage Engine:** Implements a custom file format using Slotted Pages and a Freelist manager for efficient disk I/O and space reclamation.

* **In-Memory B-Tree Index:** Rebuilt entirely in memory on startup and persisted at runtime with every change. Handles querying and searching.

* **Smart Compression Pipeline:** Implements heuristic routing. Tiny payloads bypass compression, medium payloads route to **LZ4** for speed, and bigger payloads route to **Zstandard (Zstd)** for high-ratio space saving.

* **Modern Cryptography:** Powered by `libsodium`. Uses **Argon2id** for Key Derivation and **XChaCha20-Poly1305** for entry-level Authenticated Encryption with Associated Data (AEAD).

## Dependencies

To build Vaultea, you need the following installed on your system:

* **CMake** (3.19+)
* **C++17** compatible compiler (GCC, Clang, or MSVC)
* **Qt6** (Core, Widgets, Svg) - *For the reference GUI*
* **libsodium** - *Cryptography*

*Note: Abseil, LZ4, and Zstd are automatically fetched and compiled from source via CMake `FetchContent`.*

## Build Instructions (Linux)

> **Note:** The following build instructions, CMake configuration, and developer tool commands are primarily tailored for **Linux** environments. While the core C++ code and CMake structure are cross-platform, certain toolchain flags and system paths (such as those configured in `clang-uml.yml`) assume a Linux setup.

1. Clone the repository:
   ```bash
   git clone https://github.com/GoLab18/Vaultea.git
   cd Vaultea
   ```

2. Configure with CMake:
   ```bash
   cmake -S . -B build
   ```
   *(Note: The project automatically exports `compile_commands.json` for Clang-based language servers and is needed for clang-uml usage).*

3. Build the executable:
   ```bash
   cmake --build build -j $(nproc)
   ```

4. Run the application:
   ```bash
   ./build/Vaultea
   ```

## Developer Tools

### Generating Diagrams (clang-uml)

The project uses `clang-uml` to generate architecture diagrams directly from the C++ source code. A configuration file (`clang-uml.yml`) is already supplied in the repository.

1. Ensure you have run a full build at least once (Step 3 above). This guarantees that CMake generates the `compile_commands.json` database and Qt's `uic` compiler generates the necessary `ui_*.h` header files in the `build/` directory.

2. Run `clang-uml` from the project root using the supplied configuration:
   ```bash
   clang-uml -c clang-uml.yml
   ```
The output will be placed in the `diagrams/` directory as configured in the `clang-uml.yml` file.

### Generating Code Documentation (Doxygen)

The core backend API (such as `VaultEngine.h`) is annotated with standard Doxygen comments. A `Doxyfile` is already supplied in the repository. To generate the HTML reference documentation:

1. Ensure Doxygen is installed on your system (e.g., `sudo pacman -S doxygen` or `sudo apt install doxygen`).

2. Run Doxygen from the project root (it will automatically use the included `Doxyfile`):
   ```bash
   doxygen
   ```
