# Modernize step 3 — drop Boost

**Status:** DONE 2026-09-24 (branch `modernize-cpp`).
**Part of:** `tasks/smc-modernize-cpp.md`. **Next:** step 4 (ownership → `unique_ptr`).

## Done

Boost was two things (per the reference set): `boost::filesystem` (one live user) and `boost::thread`
(behind the never-enabled `SMC_RENDER_THREAD_TEST`). Both removed; the game compiles, links, and
its image builds with Boost entirely absent.

- **`boost::filesystem` → `std::filesystem`** in `core/filesystem/filesystem.cpp`: swapped the
  `namespace fs =` alias and the include, and the file-type enum names (`fs::regular_file` →
  `fs::file_type::regular`, `symlink_file` → `symlink`, `directory_file` → `directory`). Everything
  else (`status`, `remove_all`, `create_directory`, `create_directories`, `directory_iterator`,
  `is_directory`, `temp_directory_path`) is API-compatible. GCC 16 links `std::filesystem` from
  libstdc++ (no `-lstdc++fs` needed). `filesystem.h` had no Boost.
- **`boost::thread` / the threaded renderer removed** from `video.{h,cpp}`: the `<boost/thread>`
  include, the `m_render_thread` member and its ctor init, `Render_From_Thread`,
  `Make_GL_Context_Current`/`Inactive`, and the `threaded` branch of `cVideo::Render` (now
  `Render(void)`, single-thread only). The `#ifdef SMC_RENDER_THREAD_TEST` in `main.cpp` collapsed
  to the plain `pVideo->Render()`. **This also completes the render-thread block that
  `smc-remove-dead-code.md` deferred to this step.**
- **Build config:** removed the three `AC_CHECK_LIB([boost_*])` from `configure.ac` and
  `-lboost_system` from the mac `LIBS`; removed `boost-devel` from `entrypoint/01-install-base.sh`.
  (Left the shared sandboxes' `boost-devel` alone — they are general dev boxes.)

## Verification

`build.sh game` compiles and links with no Boost, no undefined references. Full `make image` builds
green with `boost-devel` NOT installed — the definitive proof the dependency is gone.

## Left for later (minor, noted in the umbrella)

- `cVideo::Render_Finish` is now an empty no-op; its ~11 call sites are dead cruft to sweep.
- `pRenderer_current` (the double-buffer for the removed threaded path) is now allocated in
  `main.cpp` but never rendered — removable with its allocation/free.
- The 5 `// todo : use boost::algorithm` comments in `property_helper.h` (never acted on) — dropped.
