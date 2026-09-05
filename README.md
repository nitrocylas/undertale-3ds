<div align="center">

# UNDERTALE on the 3DS

undertale, rebuilt from scratch to run natively on a nintendo 3ds.

its written in C with devkitpro (libctru + citro2d) and fed the real games assets that i
pulled out of my own copy. two screens, actual pixels, running on homebrew hardware.

![Platform](https://img.shields.io/badge/platform-Nintendo%203DS-C0392B)
![Built with devkitPro](https://img.shields.io/badge/built%20with-devkitPro-2E86C1)
![Language](https://img.shields.io/badge/language-C-555555)
![Renderer](https://img.shields.io/badge/graphics-citro2d-8E44AD)
![Status](https://img.shields.io/badge/status-work%20in%20progress-E67E22)
![License](https://img.shields.io/badge/license-MIT%20(engine%20code)-27AE60)

<br>

<img src="docs/flowey.png" width="360" alt="Flowey cutscene running on 3DS">

*flowey, on hardware. top screen is the world, bottom screen is the dialogue box*

</div>

> ⚠️ **heads up — this is NOWHERE near finished.** its very much a work in progress. right now
> its basically the intro, the menu/naming screen and only the first couple ruins rooms + the
> flowey encounter. theres no real battle system yet, no save, most of the game just isnt built.
> dont expect to actually play through undertale on it, atleast not yet.

---

## what this actually is

theres no magic converter that turns the PC copy into a 3ds rom, so dont go looking for one.
undertale is a gamemaker game and the 3ds has no gamemaker runtime. so instead this thing
**re-implements the engine** from scratch in C and just drives it with the real assets. its a
big job so im building it in vertical slices, one screen at a time, that way progress is always
visible and if one scene breaks it cant take the others down with it.

its a personal use port. none of the extracted game content goes into git (see [legal](#legal)
below) — the repo is the *engine*, you rebuild the assets from your own copy.

## screenshots

all of these are the actual build running in the azahar emulator (software renderer), grabbed
straight off the two 3ds screens.

|  |  |
|:--:|:--:|
| <img src="docs/title.png" width="330" alt="Title and instructions"> | <img src="docs/name-entry.png" width="330" alt="Name the fallen human"> |
| title + instructions, the `UNDERTALE` intro menu | "name the fallen human" — the whole naming grid |
| <img src="docs/overworld.png" width="330" alt="Overworld walking"> | <img src="docs/flowey-ready.png" width="330" alt="Flowey pellets"> |
| overworld, frisk walks around. menu lives on the touch screen | the trap.. *"Are you ready? Move around!"* |
| <img src="docs/flowey-love.png" width="330" alt="Flowey LOVE dialogue"> | <img src="docs/flowey-pellets.png" width="330" alt="friendliness pellets"> |
| talking face dialogue + typewriter | *"...little white 'friendliness pellets.'"* |

## the one big design rule — use BOTH screens

the 3ds has two screens so i wanted to actually use them to declutter, not just stack the game on
one and waste the other. undertale renders at 320x240 native.

- **top screen (400x240):** the game world / battle arena only. native 320x240 centered with 40px
  bars on the sides (pixel perfect), keeps the play area clean.
- **bottom screen (320x240, touch):** all the dialogue boxes, inventory and menus (fight / act /
  item / mercy, save screen etc). its the touch screen so menus end up tappable which is nice.

the intro storyboard is the one exception — the slide art is the "world" (top) and the narration
text goes on the bottom.

## building it

you need the devkitpro toolchain (devkitARM + libctru + citro2d) and your own copy of undertale for
the assets.

```powershell
# build the .3dsx (from the repo root)
powershell -File build.ps1
```

- **testing on PC:** [azahar](https://github.com/azahar-emu/azahar/releases) (the maintained citra
  successor). just drag the `undertale-3ds.3dsx` it spits out onto it.
  - if azahar hangs on launch (happens alot over remote desktop) set the renderer to **software**:
    in `%APPDATA%\Azahar\config\qt-config.ini` under `[Renderer]`, `graphics_api=0` and
    `async_shader_compilation=true`.
  - azahar keys: `START`=M, `A`=A, `B`=S, d-pad=T/F/G/H, circle pad=arrow keys.
- **real hardware:** needs a homebrewed 3ds (luma3ds / CFW). stock consoles wont run these builds,
  no way around it.
- **audio** needs the 3ds dsp firmware (`dspfirm.cdc`). its already there after a normal CFW setup;
  on azahar you have to dump it off a console (DSP1 homebrew -> `sd:/3ds/dspfirm.cdc`) and drop it in
  `%APPDATA%\Azahar\sysdata\dspfirm.cdc`. if its missing audio just no-ops, it wont crash.

### toolchain versions (checked sep 2026)
- devkitpro installer — https://github.com/devkitPro/installer/releases
- azahar emulator — https://github.com/azahar-emu/azahar/releases
- undertalemodtool (asset extraction) — https://github.com/UnderminersTeam/UndertaleModTool/releases (v0.9.2.0)

## the asset pipeline

assets get pulled out of your own `data.win`, converted into 3ds textures and embedded, no romfs.

1. `tools/scripts/dump_names.csx` — dumps the resource names (do it once).
2. targeted export with the UTMT cli (`UndertaleModCli.exe load <data.win> --scripts <script>.csx`).
3. `tex3ds <png> -o data/<name>.t3x` — turn the png into a 3ds texture, embedded via `bin2o`.
4. build + test.

> heads up: the extraction tools (`tools/UTMT_CLI/`, `tools/UndertaleModTool/`) and the converted
> textures (`data/*.t3x`) are all **gitignored** — grab the tools from the link up top and regen the
> textures from your own copy. only the extraction *scripts* (`tools/scripts/`) are in the repo.

## progress

built as vertical slices, following undertales real boot order.

**slice 1 — intro storyboard** (`room_introstory`)
- [x] slide 0 rendering on 3ds from an extracted `t3x` (proved the pipeline works)
- [x] full slide sequence + typewriter narration + the falling pan, skippable

**slice 2 — main menu** (`room_intromenu` / `scr_namingscreen`)
- [x] instructions screen (begin game / settings) with 3ds adapted control labels
- [x] name entry grid (A-Z / a-z, quit / backspace / done), faithful navigation, 6 char cap
- [x] confirm (no / yes) + all 27 name easter eggs (sans, asgore, chara ...)
- [x] gaster -> restart, letter shake/jitter, name zoom + fade to white, matching fade in
- [x] audio engine (stb_vorbis + ndsp), intro plays `mus_story.ogg` on loop
- [ ] menu music (`mus_menu0.ogg`) + per area bgm
- [ ] the real `fnt_main` bitmap font (rn its the 3ds system font as a placeholder)

**slice 3 — ruins overworld** (build the reusable walking engine first, then the rooms)
- [x] frisk walks, 4 direction animated
- [x] `room_area1` — the flower bed landing, follow camera, wake up, integer position filtering
- [x] pixel accurate collision from a per column walkable band generated off the floor pixels
- [x] multi room engine — a room = { bg, per column band, size, door list }, doors fade + load
- [x] flowey cutscene — the rise, full dialogue, talking faces, SOUL heart, pellets, toriels rescue
- [ ] `room_ruins1` (toriel / leaves) + wiring the next door up

**later on** — the battle system (SOUL bullet board, fight/act/item/mercy), then the rest of the
content pass.

the code is a little scene system: `game.h`/`game.c` core + `scene_intro`, `scene_menu`,
`scene_game` and `flowey.c`.

## legal

undertale is © toby fox. this is a personal use engine reimplementation, its not affiliated with or
endorsed by toby fox in any way. **no copyrighted game content is shipped in this repo** — the
sprites, audio, rooms and the converted `.t3x` textures are all gitignored and stay local to a build
made from your own copy. you need to own undertale to build something playable.

## license

the **engine code in here** (everything under `source/`, `include/`, `tools/scripts/` and the build
files) is MIT, see [LICENSE](LICENSE). that only covers my original code — it does NOT give you any
rights to undertales assets, characters, music or story, those all still belong to their owner.

## combat (the battle box)

- `source/battlebox.c` — the reusable undertale battle box. white bordered arena on the top
  screen, the red SOUL heart you move around inside it, generic bullets, HP, i-frames and a bit
  of screen shake.
- floweys intro is what drives it rn: SOUL shows up -> the friendliness pellets orbit -> they
  converge (the trap, HP goes 20 -> 1) -> "You IDIOT / DIE" -> toriels flame. all on real 3ds
  buttons (d-pad / A / B / START).
- packaged up too: APP_TITLE=UNDERTALE, icon.png is the red SOUL. runs off the homebrew launcher
  as a `.3dsx`.
