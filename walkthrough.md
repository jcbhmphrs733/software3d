# Software3D Walkthrough Story (5-8 Minutes)

This version is intentionally simple. Think of your presentation as one story with four chapters, one chapter per presenter.

## The Story In One Line

We load a 3D model, convert it into pixels on the CPU, display those pixels on screen, and let the user control the scene.

## Presenter A: Setup The Scene (0:00-2:00)

Narrative:
We begin at the app entry point, restore saved settings, and prepare the window and renderer so a scene can be shown.

Open in this order:
1. [src/main.cpp](src/main.cpp#L872)
2. [src/main.cpp](src/main.cpp#L874)
3. [src/main.cpp](src/main.cpp#L113)
4. [src/main.cpp](src/main.cpp#L154)

Simple line to say:
The app starts by restoring user state, creating a window, and preparing a texture-backed display path.

## Presenter B: Turn 3D Into Pixels (2:00-4:00)

Narrative:
Each frame, we build transforms, move vertices to screen space, and rasterize triangles with depth testing.

Open in this order:
1. [src/main.cpp](src/main.cpp#L222)
2. [src/main.cpp](src/main.cpp#L442)
3. [src/main.cpp](src/main.cpp#L448)
4. [src/main.cpp](src/main.cpp#L518)
5. [src/framebuffer.cpp](src/framebuffer.cpp#L49)

Simple line to say:
This is the core graphics pipeline where math turns mesh triangles into final visible pixels.

## Presenter C: Load Data And Shade It (4:00-6:00)

Narrative:
We ingest model and texture files, sample colors from textures, and apply lighting while drawing.

Open in this order:
1. [src/mesh/obj_loader.cpp](src/mesh/obj_loader.cpp#L6)
2. [src/texture.cpp](src/texture.cpp#L6)
3. [src/rasterizer.cpp](src/rasterizer.cpp#L9)
4. [src/main.cpp](src/main.cpp#L646)

Simple line to say:
Assets provide geometry and color data, and our rasterizer combines them with lighting to produce the final look.

## Presenter D: Interaction, UI, And Persistence (6:00-8:00)

Narrative:
The user manipulates the scene in real time, changes settings in the UI, and those settings are saved for the next run.

Open in this order:
1. [src/main.cpp](src/main.cpp#L991)
2. [src/main.cpp](src/main.cpp#L937)
3. [src/main.cpp](src/main.cpp#L592)
4. [src/AppState.cpp](src/AppState.cpp#L5)
5. [src/RecentFilesManager.cpp](src/RecentFilesManager.cpp#L53)
6. [src/main.cpp](src/main.cpp#L848)

Simple line to say:
The app is not just rendering frames, it is interactive, user-friendly, and remembers your workflow.

---

## Minimal Backup Plan (If You Run Short On Time)

Only open these six links:

1. [src/main.cpp](src/main.cpp#L872)
2. [src/main.cpp](src/main.cpp#L222)
3. [src/main.cpp](src/main.cpp#L518)
4. [src/framebuffer.cpp](src/framebuffer.cpp#L49)
5. [src/main.cpp](src/main.cpp#L592)
6. [src/main.cpp](src/main.cpp#L848)

If you cover those clearly, you still tell a complete beginning-to-end story.
