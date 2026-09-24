# Investigate: port to a modern engine (Godot) OR rewrite OpenGL to 3.3 core profile

**Status:** proposed — needs go-ahead; INVESTIGATION first (William Emerison Six
<billsix@gmail.com>, 2026-09-24)
**Priority:** 6 · **Difficulty:** 9

## BLUF

SMC renders with **fixed-function, immediate-mode OpenGL** (`glBegin`/`glEnd`, `glVertex2f`,
`glTranslatef`, the matrix stack, `GL_QUADS`/`GL_POLYGON` — see `video/renderer.cpp`), which is
deprecated and unavailable in an OpenGL core profile / on many modern drivers and unsupported by
GL ES / WebGL. Investigate two divergent futures and recommend one: **(A)** port the game to a
modern engine such as **Godot** (reuse the assets/levels, reimplement the gameplay in the engine),
or **(B)** keep the C++ game but **rewrite the renderer to OpenGL 3.3 core profile** (VBOs/VAOs,
shaders, an explicit matrix library instead of the fixed-function stack). Done (for this task) = a
reference doc comparing effort, risk, portability (Windows/AppImage/web), and asset reuse for each,
with a recommendation and a scaffolded follow-on for the chosen path.

## Context

- The rendering is immediate-mode throughout `video/renderer.cpp` (`cRender_Request` subclasses each
  `glBegin`…`glEnd`) and uses `glTranslatef`/`glScalef`/`glRotatef` + the GL matrix stack; CEGUI
  0.7.9 also uses the legacy `CEGUIOpenGLRenderer` (GLEW/GLX) — a core-profile rewrite must address
  CEGUI's renderer too (0.8+ has a modern GL3 renderer, but that is the incompatible API the game
  avoided). This interacts with the vendored-CEGUI constraint.
- Option A (Godot): the assets (pixmaps, levels in the SMC XML format, music/sounds) could be reused;
  the C++ gameplay would be reimplemented (GDScript/C#), and the level/save XML parsed or converted.
  This is effectively a rewrite of the game logic on a supported engine — large, but future-proof.
  The successor project `github.com/secretchronicles/TSC` (a fork's continuation) is prior art to study.
- Option B (GL 3.3 core): keeps all the C++ gameplay; rewrites only the renderer (batch the
  `cRender_Request` queue into VBOs, replace the matrix stack with the maintainer's own
  `pyMatrixStack`/`modelviewprojection`-style matrices or a small lib, write basic 2D shaders).
  Contained to `video/` + CEGUI's renderer, but the immediate-mode calls are pervasive in the
  request `Draw()` methods.
- Relevant maintainer work: `github.com/billsix/modelviewprojection` (modern GL / matrix pedagogy)
  is directly applicable to option B's matrix + shader approach.

## Goal

An investigation reference doc (`tasks/reference/smc/rendering-modernization-options.md` or similar)
that compares Godot-port vs GL-3.3-core-rewrite on effort/risk/portability/asset-reuse and
recommends one, then scaffolds the chosen follow-on task.

## Open questions

1. Priority of the drivers: is the goal wider portability (web/ES → favors a rewrite with a
   portable API or an engine), preservation of the exact C++ game (favors B), or a fresh modern
   codebase (favors A)?
2. Is reusing the SMC level/asset format a hard requirement, or is a one-time conversion acceptable?
