# Project TODO

> **Note:** This is a living document. Please update with new tasks, ideas, and details as development progresses.

---

## Bug Fixes

- [ ] **Log System**
  - Refactor and stabilize the logging mechanism.
  - Ensure thread safety and log rotation.
  - Add log levels (debug, info, warn, error).

- [ ] **Bootscreen Performance**
  - Investigate slow bootstrap or asset loading.
  - Profile and optimize any bottlenecks.
  - Reduce time-to-interaction at startup.

- [ ] **user_config File Overwrite Issue**
  - Fix bug where `user_config` recreates/overwrites existing configs.
  - Add checks for file existence before writing.
  - Provide a safe way to update configs without data loss.

- [ ] **FAT32 Not Registered**
  - Debug FAT32 module (mounting/registration).
  - Ensure correct integration with VFS and drivers.

---

## Storage & Filesystems

- [ ] **ATAPI Support**
  - Implement ATAPI protocol for CD-ROM/DVD device access.
  - Integrate with existing storage infrastructure.

- [x] **BMP Image Format**
  - Add loader and renderer for BMP images.
  - Verify with test images of various bit depths.

- [ ] **PNG Image Format**
  - Implement PNG decoder.
  - Support transparency and various color depths.

- [ ] **MP4 Support**
  - Implement basic MP4 parsing and playback.
  - Leverage software decoding, investigate hardware acceleration.

- [ ] **AHCI Driver**
  - Develop driver for AHCI-compliant SATA controllers.
  - Support for hotplug, NCQ, and advanced features.

- [x] **libc Implementation**
  - Integrate minimal standards-compliant libc.
  - Document supported functions and limitations.

- [ ] **emez: Kernel Stub System**
  - Design & implement "emez" (Emex’s Zipped Kernel), inspired by vmlinux stubs.
  - Enable booting/compression for faster load/testing.
  - Provide generation tools/scripts.

- [x] **procfs**
  - Implement process filesystem for kernel/process info access.
  - Ensure compatibility with debugging tools and utilities.

- [ ] **ext2 Filesystem**
  - Implement read/write support.
  - Integrate with VFS layer.

- [ ] **ext4 Filesystem**
  - Implement read-only support first, later advance to write support.
  - Handle journaling, extents, and 64-bit features.

---

## Graphical Interface & User Experience

- [ ] **Window System/Server**
  - Core windowing server: composition, input routing, application management.
  - IPC mechanism between apps and server.
  - Hardware-accelerated drawing support.

- [ ] **Desktop Environment**
  - Build a standard environment (desktop, launcher, panels, wallpaper).
  - Provide basic applets and settings support.

- [ ] **Window Manager**
  - Develop window tiling, decorations, focus, stacking.
  - User-configurable settings (shortcuts, decorations, behavior).

- [ ] **Theme Manager (Upgrade)**
  - Expand theme definition (colors, icons, widgets, fonts).
  - Implement theme package support and selector UI.

- [ ] **Font Manager (Upgrade)**
  - Add support for font discovery, preview, fallback, and internationalization.
  - Cache rendering for performance.

- [ ] **Bootscreen Manager (Upgrade)**
  - Make bootscreen assets configurable.
  - Add support for progress animation, logo customizations, tips/messages.

---

## GENERAL NOTES

- Add regression tests for each major subsystem/fix.
- Prioritize clear documentation for new features.
- For completed items: review and update this file regularly.

