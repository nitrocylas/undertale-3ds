// Slice 3a export: Frisk's 4-direction overworld walk sprites (all frames).
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;

string sprDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\sprites";
Directory.CreateDirectory(sprDir);

string[] want = { "spr_maincharad", "spr_maincharau", "spr_maincharal", "spr_maincharar" };
using (var worker = new TextureWorker())
{
    foreach (var name in want)
    {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == name);
        if (spr == null) { Console.WriteLine("MISSING " + name); continue; }
        for (int i = 0; i < spr.Textures.Count; i++)
        {
            var pi = spr.Textures[i]?.Texture;
            if (pi != null) worker.ExportAsPNG(pi, Path.Combine(sprDir, $"{name}_{i}.png"), null, false);
        }
        Console.WriteLine($"{name}: {spr.Textures.Count} frames, {spr.Width}x{spr.Height}, origin {spr.OriginX},{spr.OriginY}");
    }
}
