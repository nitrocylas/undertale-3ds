using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;
string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\sprites";
Directory.CreateDirectory(dir);
using (var w = new TextureWorker()) {
    var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == "spr_spinbullet");
    if (spr != null) {
        for (int i = 0; i < spr.Textures.Count; i++)
            if (spr.Textures[i]?.Texture != null)
                w.ExportAsPNG(spr.Textures[i].Texture, Path.Combine(dir, $"spr_spinbullet_{i}.png"), null, false);
        Console.WriteLine($"spr_spinbullet: {spr.Textures.Count} frames {spr.Width}x{spr.Height}");
    } else Console.WriteLine("missing spr_spinbullet");
}
