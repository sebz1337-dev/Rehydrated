# Vanilla 17339 data-model audit

**Date:** 2026-11-22 · **Scope:** the 10 `TODO: clean up entries that didn't exist in 17339` sites.

## Decision: keep all entries as-is

| file | sites | decision |
|---|---|---|
| `src/game/types.h` | `ItemLabel` (386), `EffectType` (569), `RevivalType` (696) | kept |
| `src/game/tags.h` | `ActorTag` (3), `ActorAnimationTag` (540) | kept |
| `src/game/game_balance.h` | `GameBalanceType` (6) | kept |
| `src/game/network/game_message.h` | `QuitReason` (144), `LogEventType` (263), `DeterministicPathType` (1044) | kept |

**Why:** these enums were assembled from later-version references. The repo has no 17338/1.0.8 header or binary to compare against, so there is no evidence which specific enumerators are later additions. Removing an entry would either break code that names it or silently renumber everything after it — worse than keeping a harmless extra name. The enums are used for parsing and printing guest data; an unused C++ name does not change what the guest sends.

**Future evidence:** `src/recomp/static_data_dumper.cpp` now dumps `SymbolicEntry` tables (name/value pairs) from the 17339 XEX into `_TypeDescriptors.bin` (43 tables, 259 entries, verified 2026-11-22). A future pass can cross-reference those names against these enums and remove only the entries that are provably absent.

## Stretch task deferred

`src/game/network/game_connection.h:18` (`XPooledList` template) is deferred: it is a structural cleanup of a type the guest server actually uses, with no fidelity gain and a real risk of breaking message layout. Revisit only with a repro that needs it.

**Verification:** comment-only change; `win-amd64-release` build exit 0, smoke alive 15 s and exit 0, 0 new `[error]` lines. `not tested in game`.
