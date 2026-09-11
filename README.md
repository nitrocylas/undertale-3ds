<div align="center">

# UNDERTALE on the 3DS

im rebuilding undertale so it runs on an actual nintendo 3ds. two screens, real pixels, running
on homebrew.

![Platform](https://img.shields.io/badge/platform-Nintendo%203DS-C0392B)
![Language](https://img.shields.io/badge/made%20in-C-555555)
![Status](https://img.shields.io/badge/status-work%20in%20progress-E67E22)
![License](https://img.shields.io/badge/license-MIT%20(my%20code)-27AE60)

<br>

<img src="docs/running-on-3ds.png" width="560" alt="Undertale port running on a real 3DS">

**its running on a real 3ds!!**  thats my actual console, not an emulator. and it runs super
smooth too, no lag at all which im really happy about.

*(the whole ruins area is complete except toriels boss fight snowdins mostly ported in now as well)*

</div>

> ⚠️ **this is nowhere near finished.** its a hobby project and still really early. right now you
> got the intro, the menu, almost the whole ruins area and the flowey + toriel bit. no battle
> system past flowey, no saving, and toriel is the only character in so far. so ya, dont expect to
> play the whole game yet.
>
> also since it runs this smooth i might even try doing a normal ds port too at some point, not just
> the 3ds one. no promises but it could be possible.

---

## wait what is this

you cant just turn the PC copy of undertale into a 3ds game, theres no button for that. undertale
was made in gamemaker and the 3ds cant run gamemaker stuff. so instead im just rebuilding the game
myself in C and feeding it the real sprites and sound from my own copy.

so to be clear this is a **native port**, its all redone from the ground up and NOT running through
some emulator. thats why it runs so smooth, most likely a full 60 fps on a normal 3ds. i cant say
that for 100% sure yet though, once i get some of the bigger fights in like sans or asriel with
their crazy amount of bullets flying around, we'll see how much that hits the framerate. but so far
its buttery.

im doing it one screen at a time so theres always something working to look at, and if one part
breaks it doesnt take everything else down with it.

none of the actual undertale files go in here (see [legal](#legal)), this repo is just my code. you
bring your own copy of the game.

## screenshots

all real, straight off the two 3ds screens (running in the azahar emulator).

**the ruins so far**

|  |  |  |
|:--:|:--:|:--:|
| <img src="docs/ruins1.png" width="230" alt="Ruins 1"> | <img src="docs/ruins3.png" width="230" alt="Ruins 3"> | <img src="docs/ruins6.png" width="230" alt="Ruins 6"> |
| the leaf pile room | the little bridge | long hallway |
| <img src="docs/ruins-sign.png" width="230" alt="Ruins sign"> | <img src="docs/toriel-rescue.png" width="230" alt="Toriel"> | <img src="docs/overworld.png" width="230" alt="Overworld"> |
| reading a sign | **toriel shows up!** | walking around |

**newer stuff (end of the ruins + a peek at snowdin)**

|  |  |  |
|:--:|:--:|:--:|
| <img src="docs/napstablook.png" width="230" alt="Napstablook"> | <img src="docs/home.png" width="230" alt="Toriel's home"> | <img src="docs/sans-papyrus.png" width="230" alt="Sans and Papyrus"> |
| napstablook + the dapper blook | toriels home | **sans and papyrus** in the snow :) |

<p align="center"><img src="docs/mirror.png" width="230" alt="the mirror, its you"><br><em>the mirror in toriels house.. "its you!"</em></p>

**the start of the game**

|  |  |
|:--:|:--:|
| <img src="docs/title.png" width="300" alt="Title screen"> | <img src="docs/name-entry.png" width="300" alt="Naming screen"> |
| the title + how to play | naming the fallen human |
| <img src="docs/flowey-love.png" width="300" alt="Flowey talking"> | <img src="docs/flowey-ready.png" width="300" alt="Flowey pellets"> |
| flowey being flowey | *"Are you ready? Move around!"* |

## the two screen thing

the whole point of doing it on a 3ds is you get two screens, so im actually using both instead of
cramming everything on one.

- **top screen:** just the game world, kept nice and clean.
- **bottom screen:** all the talking, menus and inventory. and since its the touch screen you can
  tap the menus which is pretty handy.

## can i play it

not really yet, so im not gonna post build/install steps for now. theres just not enough here to
be worth it, youd basically get a demo. once its actually playable ill write up how to build it and
put it on a real 3ds. for now this is more of a "heres my progress" kinda thing.

## whats done / whats next

(theres also a [changelog](CHANGELOG.md) if you wanna see everything ive added over time)

**working:**
- [x] the intro story slides
- [x] title screen + the full naming screen (with all the name easter eggs like sans, asgore...)
- [x] walking around, animated, with proper collision so you dont walk through walls
- [x] a bunch of the ruins rooms hooked together with doors
- [x] the whole flowey intro (the friendliness pellets trap) and toriel saving you
- [x] a basic battle box (the little arena where you dodge stuff)

**not yet:**
- [ ] sound, theres no audio in it at all right now. not a big deal yet, ill get to it later
- [ ] the rest of the ruins + actual npcs (toriel is the only one in right now)
- [ ] a real battle system (fight / act / item / mercy)
- [ ] saving
- [ ] music, and the real undertale font (using a placeholder for now)

## 3ds exclusive stuff (the fun part)

since im redoing the whole thing anyway im not just gonna do a straight copy, i wanna add some stuff
thats only in this 3ds version. rough ideas so far:

- a whole new **boss** that isnt in the original
- some new **mechanics** that use the two screens / touch screen
- MAYBE a new npc or two if it fits
- other little extras, we'll see what i come up with

nothing set in stone yet but thats the plan, make it its own thing a bit not just a port.

## legal

undertale is © toby fox, im not affiliated with him at all, this is just a fan thing i make for fun.
none of the games sprites, music or rooms are in this repo, they stay on my pc. you need to own
undertale yourself to actually build a playable version.

## license

my code in here is MIT (see [LICENSE](LICENSE)). that only covers the code i wrote, it does NOT give
you any rights to undertale itself, thats all toby fox's.

## support

if you like this and wanna help me keep working on it you can toss me a couple bucks on ko-fi, no
pressure at all. thanks either way :)

[![ko-fi](https://img.shields.io/badge/buy%20me%20a%20coffee-ko--fi-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/nitrocylas)
