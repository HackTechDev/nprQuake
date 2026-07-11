# NPRQuake

Non-PhotoRealistic rendering port of Quake (GLQuake) for Linux, built around a **hot-swappable renderer plugin system**. Rendering styles live in `.so` shared libraries that can be swapped at runtime from the in-game console without restarting the game.

Video and input were ported from GLX to SDL to fix mouse/keyboard focus issues on modern desktops.

## Renderers

| Plugin | Style |
|--------|-------|
| `dr_default.so` | Standard GLQuake look (default at startup) |
| `sketch.so` | Sketchy hand-drawn outlines |
| `bprint.so` | Blueprint / technical drawing style |
| `brush.so` | Brushstroke painterly style |

## Requirements

- GCC, GNU Make
- SDL 1.2 (`libsdl1.2-dev`)
- OpenGL / Mesa (`libgl1-mesa-dev`)
- Quake retail data files (`id1/pak0.pak`, `id1/pak1.pak`)

## Build

```sh
make          # release build
make debug    # debug build
make clean    # clean
```

Outputs land in `build/`: `glquake.sdl` engine binary and `*.so` renderer plugins.

## Setup & Run

```sh
cd build/
# Place (or symlink) Quake's id1/ data directory here
ln -s /path/to/quake/id1 id1

./nprquake.sh
```

The launch script sets the keyboard layout to US while the game runs and restores it on exit. It launches at 640×480 with sound disabled.

## Switching Renderers

Open the in-game console (`` ` ``) — the keyboard is QWERTY while the game is running:

```
r_load dr_default   # default renderer
r_load sketch.so    # sketchy NPR style
r_load bprint.so    # blueprint style
r_load brush.so     # brushstroke style
```

Renderers are loaded with `dlopen` at runtime; the old renderer is shut down cleanly before the new one initialises.

## Architecture

```
NPRQuakeSrc/          Engine source (GLQuake + plugin loader)
  gl_rmisc.c          dlopen/dlsym renderer loader, global fn-pointer table
  dynamic_r.h         Plugin ABI: EXPORT symbols + dr_Set_* injectors

dynamic_r/            Renderer plugin sources
  dr_default.c        Default renderer
  sketch.c            Sketch renderer
  bprint.c            Blueprint renderer
  brush.c             Brush renderer
  glquake.h           Copy of engine headers for plugin compilation

build/                All build outputs (flat directory)
  glquake.sdl         Engine binary
  *.so                Renderer plugins
  nprquake.sh         Launch script
  id1/                Quake data (not included — provide your own)
```

**Plugin contract:** after `dlopen`, the engine calls `dr_Set_*` functions to inject its own function pointers (GL wrappers, math utilities, etc.) into the renderer, then calls `dr_Init()`. Each frame it calls the renderer's exported draw functions. On unload it calls `dr_Shutdown()`.

## 64-bit Portability Notes

This codebase was written for 32-bit x86. The following fixes were applied to run on x86_64 Linux:

- Removed duplicate global definitions of `skytexturenum` and `sb_lines`
- Added `-no-pie` to linker flags (`sys_dosa.s` uses 32-bit absolute addressing)
- Fixed pointer-difference overflow in `sv_main.c` when storing world model and map name as QuakeC string offsets (`ED_NewString` now copies them onto the hunk so the offset fits in 32 bits)
- Fixed `BLOCK_WIDTH`/`BLOCK_HEIGHT` in `dynamic_r/glquake.h` (cast to `unsigned char` made 256 evaluate to 0)
