// Export the first room's background + Frisk's lying/fall sprites for the wake-up.
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;

string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\room";
Directory.CreateDirectory(dir);

using (var worker = new TextureWorker())
{
    // Background image bg_firstroom.
    foreach (var bgName in new[] { "bg_firstroom" })
    {
        var bg = Data.Backgrounds.FirstOrDefault(b => b.Name?.Content == bgName);
        if (bg?.Texture != null) {
            worker.ExportAsPNG(bg.Texture, Path.Combine(dir, bgName + ".png"), null, false);
            Console.WriteLine($"{bgName}: {bg.Texture.SourceWidth}x{bg.Texture.SourceHeight}");
        } else Console.WriteLine("MISSING bg " + bgName);
    }
    // Frisk lying on the flowers (wake-up).
    foreach (var sname in new[] { "spr_mainchara_lie", "spr_mainchara_fall" })
    {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == sname);
        if (spr == null) { Console.WriteLine("MISSING " + sname); continue; }
        for (int i = 0; i < spr.Textures.Count; i++)
            if (spr.Textures[i]?.Texture != null)
                worker.ExportAsPNG(spr.Textures[i].Texture, Path.Combine(dir, $"{sname}_{i}.png"), null, false);
        Console.WriteLine($"{sname}: {spr.Textures.Count} frames {spr.Width}x{spr.Height}");
    }
}
