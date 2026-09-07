# changelog

just me keeping track of what ive added, newest stuff at the top. dates are roughly when i got it
working. this is a hobby project so its not on any real schedule, i just add stuff when i can.

## sep 2026

**running on real hardware**
- it boots and runs on an actual 3ds now, not just the emulator!! and it runs super smooth, looks
  like a full 60 fps. really happy with how well it holds up.
- confirmed the native port approach works, no emulation, all my own C code.

**toriel + almost the whole ruins**
- toriel shows up and saves you from flowey (shes the only character in so far).
- got most of the ruins rooms built and hooked together with doors, the leaf pile room, the little
  bridge, the long halls, the sign puzzles etc.
- you can read the signs/plaques around the ruins.

**flowey + the battle box**
- the whole flowey intro is in, the "friendliness pellets" trap, the talking faces, the typewriter
  text, all of it. then toriel comes in.
- built a basic battle box (the little arena where the SOUL dodges stuff), which ill reuse for the
  real fights later.

**the overworld engine**
- frisk walks around, animated in all 4 directions.
- proper collision so you actually stop at walls instead of walking through them.
- a multi room system, each room has its own background + doors that fade and load the next room.

**the menu + naming screen**
- the title screen and the how to play / instructions screen.
- the full "name the fallen human" grid, works just like the real one, 6 letters max.
- all the name easter eggs are in (sans, asgore, chara and the rest of them).

**the intro**
- the intro story slides with the typewriter narration, skippable.

**getting it building**
- got the whole toolchain set up and the first little citro2d app building and linking.
- basically the "does anything show up on the 3ds at all" milestone, and it did.

## whats coming

not done yet but on the list: sound (theres none right now), the rest of the ruins + real npcs, an
actual battle system (fight/act/item/mercy), saving, and the fun stuff, some 3ds exclusive content
like a brand new boss and mechanics that use both screens. we'll see how far i get.
