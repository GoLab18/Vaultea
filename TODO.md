## Features

- Robust password generation with strength checking.
- Cloud-based synchronization (WebDAV, S3 etc.).
- Vault Settings Context at creation (KDF iterations configuration, explicit compression overrides etc.).
- Auto-Lock timer based on system inactivity/mouse movement.

## Fixes & Improvements

- Result types instead of bools returned from VaultEngine-level methods.
- Properly spaced out form for entries creation.
- Icons instant color adjustment on theme swap using `QEvent::PaletteChange`.
- Serialization with Endian-safe buffer writes.
- Page-level checksums for detecting disk-rot/corruption.
- TUI port as a frontend alternative to Qt.
