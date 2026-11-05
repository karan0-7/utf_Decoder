# CLAUDE.md

This file gives Claude Code context for working in this repository.

## Project

A from-scratch UTF-8 decoder in C++. It reads a file byte by byte, classifies each byte as a leader or follower, extracts the payload bits, and rebuilds Unicode code points by hand, without a library. The decoded code points are returned as a `std::vector<int>` of decimal values. The project is in progress. It decodes valid UTF-8 correctly, but strict validation is not done yet, and `decode()` still prints debug lines.

## Layout

- `main.cpp` is the entry point. It calls `decode("input.txt", characters)` and prints each code point as `Decimal value: N`.
- `decoder/decoder.h` declares `void decode(std::string fileName, std::vector<int> &characters)`. It has `#pragma once`.
- `decoder/decoder.cpp` holds all the logic:
  - `LeaderByte` class: classifies a leader byte using mask/pattern checks (1-4 byte forms, plus an `INVALID` state). `processLeaderByte` returns `LeaderByteInfo{isValid, numberOfFollowers, dataBits}`.
  - `FollowerByte` class: checks the `10xxxxxx` pattern and extracts the low 6 bits. `processFollowerByte` returns `FollowerByteInfo{isValid, dataBits}`.
  - `BYTE_TYPE` enum plus the globals `currentByteType` and `totalFollowerBytes`. These drive the LEADER/FOLLOWER state machine.
  - `generateCharacter(bytes)` combines the leader bits and follower bits with `sum = (sum << 6) | bits`.
  - `decode()` opens the file in binary mode, loops over bytes, and appends code points to `characters`.
- `input.txt` is the sample input.
- `decoder.o`, `main.o` and `main` are untracked build artifacts. There is no `.gitignore`.

## Build and run

There is no build system (no Makefile or CMake). Build from the repo root:

```bash
g++ -std=c++17 -Wall -Wextra main.cpp decoder/decoder.cpp -o main
./main    # reads input.txt from the current directory
```

`input.txt` is resolved relative to the working directory, so run from the repo root. There are no automated tests. To check behavior, write test bytes to `input.txt` with `printf` and compare the output with Python (`open('input.txt','rb').read().decode()`).

The build currently has no warnings. Keep it that way, and prefer `-Wall -Wextra` so missing returns and truncating conversions are caught.

## Behavior and design decisions

- **Invalid leader byte** (stray follower `80`-`BF`, or `F8`-`FF`): the byte is skipped and one `U+FFFD` (65533) is appended to `characters`.
- **Invalid follower byte** in the middle of a character: the owner chose to **skip** it. The decoder appends one `U+FFFD`, discards the partial character and the offending byte, and returns to LEADER state. The offending byte is not re-read as a leader. Do not change this unless asked.
- **Input ends mid-character:** one `U+FFFD` is appended and the state is reset, after the loop.
- **ASCII (one-byte leader):** appended directly with no followers.
- Verified examples: `A é € 𝓗` gives `65, 233, 8364, 120023`. `𠜎` (`F0 A0 9C 8E`) gives `132878` (`U+2070E`).

## Fixed bugs (history)

These were found and fixed. They are listed so the same mistakes are not repeated.

1. **Header and definition disagreed** (`int` vs `void`). Now both are `void`, and the header has `#pragma once`.
2. **`processLeaderByte` rejected ASCII** because `ONE_BYTE` was enum value 0 and the check was `!type`. Now `ONE_BYTE = 1` and the check is `type == INVALID`.
3. **`getLeaderByteType` had no fall-through return.** Now it returns `INVALID`.
4. **Missing returns in `switch` functions** (`extractLeaderByteDataBits`, `getNumberOfFollowersLeaderByte`). Now each handles `INVALID` explicitly and returns a fallback (`0` and `-1`). There is no `default:` label, so `-Wswitch` still flags new enumerators.
5. **Three-byte data mask** was `0x1F`. Now `0x0F`.
6. **Signed byte types.** `int8_t` was replaced by `uint8_t` for bit work. Debug output casts to `int` before printing, because `cout << uint8_t` prints a character.
7. **`characterBytes` was never cleared**, so the second multi-byte character was built from the previous character's bytes as well (for example `é` then `€` gave 61087916). Now it is reset after each character.
8. **Leader bits pushed to the output** (`characters.push_back(dataBits)` instead of `characterBytes.push_back`) broke every multi-byte character. Fixed.
9. **`0xFFFD` pushed into the `uint8_t` buffer.** It truncated to `0xFD` and corrupted the next character. Now it is pushed to `characters`.
10. **Character cut off at end of file** was dropped silently and left the state in FOLLOWER. Now handled after the loop.
11. **Debug output cleanup.** The raw-character prints and the `ppppppppp` line in `generateCharacter` were removed. The leader and follower info lines now use `std::boolalpha` and an integer cast, with the missing `'\n'` fixed.

## Open issues

1. **No validation of the decoded value** (`generateCharacter` and `decode`). These are accepted as valid:
   - Overlong encodings, for example `C0 80` gives `0`. Minimums are: 2 bytes `U+0080`, 3 bytes `U+0800`, 4 bytes `U+10000`.
   - Surrogates `U+D800`-`U+DFFF`, for example `ED A0 80` gives `55296`.
   - Values above `U+10FFFF`, for example `F4 90 80 80` gives `1114112`.
   - Leaders `C0`, `C1` and `F5`-`F7` pass the leader check but are never valid.

   A fix needs the number of bytes used, plus a range check after `generateCharacter`. A failed check should be handled like other invalid input (one `U+FFFD`).
2. **A file that fails to open is ignored.** The `if (!file)` block in `decode()` is empty, so a missing `input.txt` gives no output and exit code 0. `decode` returns `void`, so the caller cannot detect it.
3. **Global state.** `currentByteType` and `totalFollowerBytes` are globals. The end-of-file reset cleans them up, but an early return or exception could leave them dirty for the next `decode()` call. Making them locals inside `decode()` is safer.

## Minor issues

- `<vector>` is included twice in `decoder.cpp`, `<unordered_map>` is unused, and `<cstdint>` is missing (`uint8_t` only works through another header).
- `std::string currentChar` in `decode()` is unused.
- `getLeaderByteType(uint8_t &byte)` takes a non-const reference it does not need. `generateCharacter` copies its vector by value, so it should take `const std::vector<uint8_t> &`.
- `decode()` prints leader and follower debug lines for every byte. This gets in the way once clean output is wanted.
- `decoder.h`, `decoder.cpp` and `main.cpp` have no trailing newline.
- `file.get(c)` reads one byte at a time. This works, but is slow for large files.

## Conventions

- Allman brace style, 4-space indent, `camelCase` for functions and variables, `UPPER_CASE` for enum values.
- Use `uint8_t` for raw bytes and bit masks. Cast to `int` before printing.
- Keep the decoder dependency-free (standard library only).
- Git: `main` is the only branch, with a single commit so far. `main.cpp` and the decoder files have uncommitted edits. Don't commit build artifacts.
