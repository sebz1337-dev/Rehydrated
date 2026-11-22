# STATE_100 — measured gaps toward 100%

**Date:** 2026-11-22 · **Repo:** `E:\dev\d3\Rehydrated` · **Scope:** decomp-only; PC proxy out of scope.
All numbers measured from the current worktree, not recalled.

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

## Next actions (ordered)

1. **p0_4**: commit + push this file at the phase boundary.
2. **p1**: build Ghidra pseudocode export tooling, then name functions in batches, starting with the 12 deterministic gaps and the 364 guest imports / hook targets.
3. **p2**: work the 90 called stubs — implement or document each; rebuild the SDK patch.
4. **p3/p4/p5**: re-verify data, get owner playtest, final audit.

**Open decision for the owner:** how to redefine "100% function coverage" so it is tractable (playtest-reachable functions vs all 114,931 emitted functions).
