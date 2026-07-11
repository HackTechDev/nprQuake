# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Is

NPRQuake is a Non-PhotoRealistic rendering port of Quake (GLQuake) for Linux. The key modification over standard GLQuake is a dynamic renderer plugin system: rendering code lives in hot-swappable shared libraries (`.so` files) that can be loaded at runtime from the in-game console with `r_load <renderer>`.

Video/input was ported from GLX to SDL to fix mouse/keyboard focus issues.

## Build Commands

From the repo root:

```sh
make          # release build (all: engine + dynamic renderers)
make debug    # debug build
make clean    # clean all build artifacts
```

The root `Makefile` delegates the engine to `NPRQuakeSrc/Makefile` (`build_release` target) and compiles the `.so` renderers directly from `dynamic_r/*.c`.

Outputs land in `build/`: `glquake.sdl` binary, `*.so` renderer plugins, `nprquake.sh` launch script.

## Running

```sh
cd build/
# Link or copy Quake's id1 data directory here first
./nprquake.sh              # sets keyboard to US, launches, restores to FR on exit
```

The launch script passes `-nosound -width 640 -height 480`. To load a renderer from the in-game console (keyboard is QWERTY while game runs):

```
r_load dr_default      # default renderer
r_load sketch.so       # sketchy NPR style
r_load bprint.so       # blueprint style
r_load brush.so        # brushstroke style
```

## Architecture

### Engine (`NPRQuakeSrc/`)

Standard GLQuake source compiled as a single ELF binary (`glquake.sdl`). The only significant additions over vanilla GLQuake are:

- **`gl_rmisc.c`** — dynamic renderer loading/unloading via `dlopen`/`dlsym`. Holds global function pointers for all rendering operations that renderers can implement.
- **`dynamic_r.h`** — the plugin ABI: declares the `EXPORT`-ed symbols each renderer `.so` must provide, plus the `dr_Set_*` setter functions the engine calls to inject its own function pointers into the renderer.

The `r_load` console command triggers `dlopen` on the named `.so`, retrieves all `dr_Set_*` function pointers via `dlsym`, calls them to inject engine callbacks, then calls `dr_Init()`.

### Dynamic Renderers (`dynamic_r/`)

Each renderer (e.g. `dr_default.c`, `sketch.c`, `bprint.c`, `brush.c`) is compiled as a position-independent shared library with `-D_DLL_BUILD -shared`. The directory contains a copy of all engine headers so renderers can reference Quake types without linking against the engine.

**Plugin contract:**
1. Engine calls `dr_Set_*` functions immediately after `dlopen` to give the renderer function pointers to engine internals (`GL_Bind`, `VectorMA`, `AngleVectors`, etc.).
2. Engine calls `dr_Init()` — renderer registers its cvars here.
3. Each frame, the engine calls the renderer's exported render functions (`R_DrawSequentialPoly`, `GL_DrawAliasFrame`, `R_DrawParticles`, etc.).
4. On unload, the engine calls `dr_Shutdown()`.

Renderers must not call engine functions before `dr_Set_*` has populated the corresponding pointer or they will segfault (NULL call).

### Build Output (`build/`)

All outputs are flat in this directory. The `.so` files must be in the same directory as `glquake.sdl` because `r_load` uses `LD_LIBRARY_PATH=.` and a bare filename.

## Key Compiler Flags

| Flag | Purpose |
|------|---------|
| `-fPIC` | Required for both engine and `.so` files (engine exports symbols; renderers are shared libs) |
| `-DGLQUAKE` | Enables the OpenGL rendering path throughout the engine |
| `-D_DLL_BUILD` | Renderer-only; gates plugin-specific code paths in shared headers |
| `-march=native` | Release builds are tuned to the build machine |
