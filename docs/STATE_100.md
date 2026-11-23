# STATE_100 — measured gaps toward 100%

**Date:** 2026-11-22 · **Repo:** `E:\dev\d3\Rehydrated` · **Scope:** decomp-only, plus owner-authorized PC proxy debugging from 2026-11-22 16:55.
All numbers measured from the current worktree, not recalled.

> **UPDATE (owner-authorized redefinition):** the literal function-coverage
> target (~110,400 semantic names) is **retired**. The new criterion is:
> every hook target, guest import, and their direct callees has a semantic
> name. Guest imports are already named `__imp__X` by the SDK registry, so
> they are excluded from the config target set. Measured 2026-11-22:
> **74 hook/callee targets**, of which 34 were already named; 8 hook-target
> names and 32 call-graph-derived names were added. Two mid-function hook
> addresses cannot be function starts (naming either breaks branch
> validation): `0x823C04A8` and, found during PC-proxy debugging on
> 2026-11-22, `0x829B8580` (split the function containing the branch target
> `0x829B8578`, which disabled `HydraHook_XServerReadUpdate_OnJoined`).
> Both are excluded with reason. **Final: 72/72 valid targets named**,
> config names 3,907 → 3,947 → 3,946 (one removed with the second exclusion).
> Target list: `E:\dev\d3\coverage_targets.txt`.

---

## 1. Function coverage — NOT 100%

| measure | number |
|---|---|
| functions emitted by codegen (`DECLARE_REX_FUNC` in `generated/default/rehydrated_funcs*.h`) | **114,931** |
| function entries in `config/functions.toml` + `config/functions_ghidra.toml` | **3,907** |
| of which deterministic `sub_` names added in Phase 3 of the prior goal | **12** |
| semantic names (from the original IDA map) | **3,895** |
| unique `sub_XXXXXXXX` placeholders in generated `rehydrated_recomp.*.cpp` | **110,431** |
| semantic-name coverage | **3.39%** (3,895 / 114,931) |
| Ghidra 12.2 auto-analysis discovered functions | 34,095, **0** with semantic names |
| codegen unresolved-branch warnings (baseline, build still exits 0) | **5** |

**Honest blocker:** the literal 100% criterion ("every emitted guest function has a semantic name") requires naming ~110,400 functions by hand. That is not tractable in this project's lifetime. A tractable redefinition needs an owner decision, e.g. 100% of functions reachable during an owner playtest, or 100% of hook targets and guest imports. Until then this dimension is measured, not achieved.

## 2. HLE coverage — NOT 100% (but now tractable)

| measure | number |
|---|---|
| unique guest imports (`DECLARE_REX_FUNC(__imp__*)`) | **364** |
| `REX_EXPORT_STUB` declarations in pinned SDK | **2,402** |
| guest imports that map to an SDK stub | **90** |
| list | `E:\dev\d3\called_stubs.txt` |

Most of the 90 are `NetDll_*` / `XNet*` (Xbox Live networking) and mic/voice exports that offline single-player likely never calls. The rest are Io/Ob/misc. 100% here means: each of the 90 is either implemented or has a documented reason it is unreachable in offline play, and no STUB lines appear in a playtest log.

## 3. Data fidelity — 100% by the defined criterion

- 0 unresolved data-model TODOs in non-network source (all remaining TODO/HACK markers are in network/PC-proxy files, out of scope).
- 10/10 former `not in 17339` sites have audit decisions in `docs/vanilla-data-audit.md`.
- `_TypeDescriptors.bin` 801,276 B (43 tables / 259 symbolic entries), `_GameMessages.bin` 5,924 B / 370 records, `_AttribDefs.bin` 71,151 B / 908 attribs / 29 compressors / 1,037 strings — all validate byte-exact with `E:\dev\d3\tools\check_all_dumps.ps1`.

## 4. Runtime / playtest — NOT MEASURABLE YET

- Launch/smoke: window alive 15 s, graceful exit 0, 2 benign `cache:\` errors, 0 new errors, 0 STUB lines.
- Input smoke: window-targeted synthetic events execute the new handlers without crash; guest interpretation unverified.
- **No owner campaign playtest has occurred.** Save/load, rendering over time, and full input behavior remain unverified. This dimension cannot reach 100% without the owner.

## 5. Tooling — 100%

- `win-amd64-release` and `win-amd64-debug` build exit 0 at final HEAD.
- `build-rehydrated.ps1` supports Configure/Build/Run/Smoke/Debug/ExtraArgs and logs a description per run.
- `check_all_dumps.ps1` verifies all three dumps; `input_smoke.ps1` exercises input handlers.
- `WORKLOG.md` and `HANDOFF.md` contain dated measurements for every prior phase.

---

## 6. PC proxy (owner-authorized follow-on, separate from the five dimensions)

Measured with a prepared 1.0.8.16603 client (`E:\dev\d3\D3Client`, MD5
`2C0A1479C17C5743C3CA6D6B148ABF82`), three byte patches applied, `Rehydrated.Bnet`
on 1119 and recomp `--server_pc_proxy_enabled=true` on 6112:

- Ports 1119 and 6112 both listening; PC client login succeeds and shows character select.
- Incoming PC `CMSG_JOIN_BNET_GAME` (PC type 17, console type 11) is decoded by
  `PCProxy::ReceiveMessages`; `m_joinRequested` gates the server's Start button.
- **Root cause of the stuck server found and fixed:** `Prefs__GetFirstTimeFlowCompleted`
  (0x8244A4A0) reads a prefs field that never persists in this environment (the
  SDK deliberately leaves `cache:` unmounted), so the Xbox start screen kept
  re-activating the safe-zone/gamma first-time flow. A hook now returns a guest
  pointer to a `u32` set to 1 ("completed") while proxy mode is enabled. The
  server then reaches Hero Creation, hosts a game after the PC client joins, and
  translates its messages to the PC client.
- **Owner-driven end-to-end result (2026-11-22):** PC client loaded into New
  Tristam with chat `Gorr says: This place reeks of death. The fallen star is
  near.` — the server's hero name was translated and delivered. Proxy log
  `recomp_20261122_201713.log`: 525 `sent` lines, 81,917 bytes to the client,
  0 invalid/truncated/corrupt warnings, TCP 6112 ESTABLISHED. Recomp log
  `rehydrated_048.log`: errors=2 (both documented benign `cache:\` lines),
  STUB=0, no crash.
- All temporary diagnostic hooks were removed; the kept source changes are the
  `0x8244A4A0` pref hook and the `0x829B8580` naming exclusion above.
- Not tested in game by the agent beyond launch/login/menu; the owner drove the
  successful session. Owner reported one `BAD DATA :)` string on screen; it is
  not present in our binaries, the PC client EXE/DLL, or the Bnet emulator source
  (likely a guest asset string). Context needs owner description.

## Next actions (ordered)

1. Commit + push the proxy fix and this updated state.
2. Owner campaign playtest log is still required to close dimensions 2 and 4.
3. Describe the context of the `BAD DATA :)` string if it recurs.
