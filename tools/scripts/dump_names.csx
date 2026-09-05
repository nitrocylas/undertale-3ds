// Dump just the NAMES of major resources so we can pick assets per vertical slice
// without exporting thousands of files. Writes text lists into extracted/_names/.
using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;

string outDir = @"C:\Users\jojoc\undertale-3ds\extracted\_names";
Directory.CreateDirectory(outDir);

void DumpList(string fileName, IEnumerable<string> items)
{
    File.WriteAllLines(Path.Combine(outDir, fileName + ".txt"), items.OrderBy(s => s));
}

DumpList("sprites", Data.Sprites.Select(x => x.Name?.Content ?? "<null>"));
DumpList("rooms",   Data.Rooms.Select(x => x.Name?.Content ?? "<null>"));
DumpList("fonts",   Data.Fonts.Select(x => x.Name?.Content ?? "<null>"));
DumpList("objects", Data.GameObjects.Select(x => x.Name?.Content ?? "<null>"));
DumpList("sounds",  Data.Sounds.Select(x => x.Name?.Content ?? "<null>"));

Console.WriteLine("NAME DUMP DONE: sprites=" + Data.Sprites.Count +
    " rooms=" + Data.Rooms.Count + " fonts=" + Data.Fonts.Count +
    " objects=" + Data.GameObjects.Count + " sounds=" + Data.Sounds.Count);
