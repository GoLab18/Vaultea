# Vaultea — Technical Specification & File Format

## 1. Terminology

* **Vault (`.vtea`)**: The physical binary file on the disk containing all encrypted data.
* **Page**: The fundamental fixed-size unit of disk I/O (defaults to 4096 bytes).
* **Slot**: A logical partition within a Data or Index page where encoded byte arrays (Processed Blobs) are stored.
* **Processed Blob**: The final byte array resulting from compression and encryption, ready to be written to a slot.

## 2. File Structure

A `.vtea` vault file is divided into three distinct segments:

1. **Preamble (Unencrypted)**
2. **Vault Header (Encrypted)**
3. **Page Region (Encrypted at the Slot level)**

### 2.1 Vault Preamble (80 Bytes)

The Preamble is strictly 80 bytes long and resides at offset `0`. It is unencrypted so the engine can verify the file and derive keys before reading the rest of the database.

| Offset | Field | Size | Description | 
| ----- | ----- | ----- | ----- | 
| 0 | `magic` | 4B | File signature: `V`, `T`, `E`, `A` | 
| 4 | `version` | 4B | Format version (Currently `1`) | 
| 8 | `uuid` | 16B | Unique identifier for the vault | 
| 24 | `salt` | 16B | Argon2id cryptographic salt | 
| 40 | `keyCheck` | 32B | HMAC-SHA256 signature to verify correct password | 
| 72 | `pageSize` | 4B | Configured page size (default 4096) | 
| 76 | `headerSize` | 4B | Size of the following encrypted VaultHeader | 

### 2.2 Vault Header

Starts immediately after the Preamble. Is always encrypted. Contains metadata essential for bootstrapping the persistence managers:

* Root Page ID for the Index
* Root Page ID for Data
* Root Page ID for the Freelist
* Global entity counters and timestamps.

### 2.3 Page Region

The remainder of the file is a continuous region divided into blocks of `pageSize`. Every page shares a common header and is categorized by its type, allowing the storage managers to track and manipulate them efficiently.

## 3. Storage Managers & Page Layout

To allow for efficient traversal and management, pages of the same type are arranged in doubly linked-list-like chains. Every page starts with a **Common Header (25 Bytes)** containing:

* `Page ID`
* `Page Type`
* `Prev Page` & `Next Page` pointers

Based on the `Page Type`, the rest of the page utilizes a specific internal layout:

### 3.1 Data and Index Pages (Slotted Layout)

Both Data and Index pages utilize a **Slotted Page Architecture**. Following the common header, the page contains:

1. **Slotted Layout Meta:** A lower boundary (grows up), an upper boundary (grows down), and an array of `Slot` definitions (Offset, Size, State).
2. **Free Space:** The dynamic gap between the lower and upper boundaries.
3. **Data Payloads:** The actual byte arrays written from the upper boundary downwards.

* **Data Pages:** Store the actual encrypted `ProcessedBlobs` (the credential and secure note payloads).
* **Index Pages:** Store the encrypted metadata (`IndexEntry`) that maps UUIDs to physical record locations. This is required to dynamically rebuild the in-memory B-Tree index on startup.

*Compaction:* When a slot is deleted, the `SlottedPageHandler` performs sliding compaction to defragment the page and recover contiguous free space.

### 3.2 Freelist and Free Pages

* **Freelist Pages:** Keep track of reclaimed page IDs that can be reused for future allocations. To save space, these pages are lazily initialized, meaning they are only allocated and linked when there are actually deleted pages to track.
* **Free Pages:** Reclaimed pages that currently hold no valid data or freelist tracking information, waiting to be repurposed as new Data, Index, or Freelist pages.

## 4. Indexing Strategy

Unlike traditional heavy relational databases (e.g., PostgreSQL) that rely on complex, disk-backed B-Trees that must handle extensive node splitting and rebalancing on disk, Vaultea uses a simplified **In-Memory B-Tree Index** powered by Abseil.

* **Startup Rebuild:** On startup, the engine scans the `Index` pages and completely rebuilds the B-Tree in memory using the persisted `IndexEntry` metadata (which maps UUIDs to physical `RecordRef` locations). Every subsequent insert/update/delete persists the metadata to the disk index pages while simultaneously updating the in-memory tree.
* **Why this approach?** Credential vaults typically contain hundreds or thousands of records, not millions. Holding the entire routing index in memory is simpler and more manageable at this scale. It completely bypasses the need for complex on-disk index balancing algorithms, drastically simplifying the storage engine's design while still providing good query performance.

## 5. Cryptography & Compression Pipeline

### Pipeline Flow:

`Raw Entry -> Compress -> Encrypt -> ProcessedBlob -> SlottedPage`

### Compression Heuristics

Compression is determined dynamically by the `DefaultCodec` to optimize CPU and space:

* **< 512 Bytes:** Bypasses compression (e.g., standard passwords).
* **512B - 64KB:** Routes to **LZ4** for real-time speed.
* **> 64KB:** Routes to **Zstd** for deep space saving.

### Cryptography

* **KDF:** `crypto_pwhash` (Argon2id) creates a 32-byte Master Key.
* **AEAD:** `crypto_aead_chacha20poly1305_ietf` encrypts the compressed payload.
* Every `ProcessedBlob` is prefixed with a unique, randomly generated 24-byte Nonce to ensure semantic security across the database.

## 6. Serialization (MVP Limitations & Future Work)

Currently, serialization uses binary `std::memcpy`.

**Known Limitations (To fix later):**

1. **Endianness:** Currently assumes a little-endian architecture (x86/ARM). Not safely portable to big-endian CPUs. Fix: Implement explicit LE/BE bit-shifting.
2. **Versioning:** The current format is rigid. Field order changes will break compatibility. Fix: Implement protobufs/flatbuffers or per-struct schema versioning.
3. **Integrity:** The `ProcessedBlob` relies on the AEAD MAC for tampering detection, but full-page checksums (xxHash) should be added to detect disk rot.
