# Replaced third-party / hand-rolled code with standard C++ where the standard has it

**Done 2026-11-07** (authorized batch; branch `std-swaps-managers-cmake`). Decisions by William Emerison
Six <billsix@gmail.com>: which swaps were left to the agent ("whatever you think would be better, and
more standard"); the XML question was moved to the parked CEGUI port.

## What was done

Five swaps, one commit each, after the F45 `std::regex` precedent. The durable record (table,
semantics, the oracle) is `tasks/reference/refactoring-to-modern-cpp.md` › "Library code → the standard
library".

1. Number↔string helper bodies → `std::to_string` / `std::to_chars` / `std::from_chars`
   (`core/property_helper.cpp`), istream semantics kept by `parse_number<T>`.
2. File helpers → non-throwing `std::filesystem` (`core/filesystem/filesystem.cpp`).
3. Screenshot/image-cache PNG writer → SDL2_image `IMG_SavePNG` (`video/video.cpp`); libpng dropped from
   `configure.ac` and `entrypoint/01-install-base.sh`.
4. `rand()`/`srand` → `<random>` (`Random_Engine`, `Random_Below` in `core/math/utilities.h`), codemod
   `tasks/adhoc/smc-replace-libraries-with-std/rand_to_random.py`.
5. `SDL_GetTicks`/`SDL_Delay` → `<chrono>` (`Ticks_Ms`, `Sleep_Ms` in `core/framerate.h`).

Not done: XML (no `std` parser; moved to `tasks/smc-cegui-0.8-port.md` step 1); `ostringstream` →
`std::format` where gettext supplies the format; the `Clamp` template in `utilities.h` (it tolerates
`min > max`, which `std::clamp` does not, so it is not a drop-in).

## A gate problem found and fixed on the way

`make image` built from a host tree's stale generated `configure` (2026-09-24, before `configure.ac`
gained the `-Werror` gate on 2026-10-18) and reused host `.o` files, so the image build had **not** been
enforcing the warning gate from this working tree. Fixed in its own commit: `.dockerignore` excludes
generated autotools files and build output; `entrypoint/build.sh` regenerates `configure` when
`configure.ac` is newer. Verified by reading the image's `src/Makefile` `CXXFLAGS` (full gate flags).

## Verification

- The number-helper oracle (`number_helper_oracle.py` + `harness_main.cpp.in`, corpus from
  `discover.sh` → `data/values.txt`): identical results on all 17,981 data values and 20,000 random
  doubles, except the accepted \|x\| ≥ 2³¹ `float_to_string` precision fix.
- Every commit: gcc and clang `-Werror` builds with 0 warnings (`clang_gate.sh`, `CXX=` selects the
  compiler; builds a scratch copy of the mounted tree in the image).
- Final (after all five): full image rebuild (by hand, `podman build --network=host`, the nested
  gotcha) builds the game under the gate; headless smoke passes (exit 137 at the 30 s deadline, clean
  `ps` audit); the image cache wrote **930 PNG files through `IMG_SavePNG`** during the smoke, each with
  a valid PNG signature.
- **Maintainer play-test items** (the smoke can't see these): the screenshot key writes a readable PNG;
  random particle colours and enemy behaviour look normal; a level with float attributes loads and saves;
  frame pacing feels the same.
