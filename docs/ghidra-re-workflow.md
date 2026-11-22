# Ghidra RE workflow (Phase 3, 2026-11-22)

The pinned SDK has no XEX file format support in Ghidra, but the recomp itself
loads the XEX into guest memory as a **PE image** (machine `0x01F2` PowerPC,
image base `0x82000000`). Dumping that image gives a file Ghidra can load.

## Pipeline

1. **Capture the image.** A temporary cvar (`dump_decrypted_image`) wrote
   `REX_IMAGE_SIZE` bytes (`0x11E0000` = 18,743,296) from guest `0x82000000`
   to `E:\dev\d3\Default_decrypted.bin`. The cvar was removed after capture.
   Note: `Memory::GetHostPtr()` applies a heap offset that lands on the host
   EXE; use `REX_KERNEL_MEMORY()->virtual_membase() + 0x82000000` instead.
2. **Verify the PE.** `e_lfanew=0x108`, signature `PE`, machine `0x01F2`,
   9 sections, image base `0x82000000`. `MZ` at byte 0 is expected.
3. **Import into Ghidra 12.2 headless** with JDK 21 and
   `GHIDRA_HEADLESS_MAXMEM=8G` (the 2G default OOMs during auto-analysis):

   ```
   analyzeHeadless.bat E:\dev\d3\ghidra_project d3 -import E:\dev\d3\Default_decrypted.bin \
     -processor PowerPC:BE:64:default -analysisTimeoutPerFile 900 -overwrite
   ```

   The correct language ID is `PowerPC:BE:64:default`, not `PPC:...`.
4. **Export / inspect** with the Java scripts in `E:\dev\d3\tools`:
   - `ExportFunctions.java` → `ghidra_functions.toml`
   - `InspectUnresolved.java` → `ghidra_unresolved.txt`

## Measured result

- Ghidra discovered **34,095** functions, but **0** user-named or otherwise
  meaningful names — every label is `FUN_...`. Ghidra cannot replace the
  3,895 semantic names already in `config/functions.toml` (they came from IDA).
- All 12 addresses in `config/functions_unresolved.toml` were inspected. Six
  have disassembly; the rest have no instruction stream in Ghidra's analysis
  (mid-function entries recorded by codegen validation). Exact evidence is in
  `E:\dev\d3\ghidra_unresolved.txt`. They are now also listed with deterministic
  names in `config/functions_ghidra.toml`, raising the map from 3,895 to 3,907.
- Switch table `0x82B02754` disassembles as `lis r11,-0x7d51` followed by the
  configured labels at `+4`, so the existing `[[switch_tables]]` entry is
  correct; no change needed.
- Codegen still prints 5 "Unresolved conditional branch" warnings and the
  `Function 0x82F41280 is 1167950 bytes` warning. All were present at baseline
  (`build_p0_4_release2.log`) and do not fail the build.

## Conclusion

The Ghidra route is useful for **verification and gap documentation**, not for
growing the semantic function map. Meaningful names still require IDA-style
manual RE. The remaining work in this direction is to name the 12 gaps in a
disassembler, not to automate more `FUN_` exports.
