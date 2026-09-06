// Extract the remaining battle expression sprites + effects for a 1:1 Flowey scene.
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;
string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\sprites";
Directory.CreateDirectory(dir);
string[] want = {
    "spr_floweysassy", "spr_floweylaugh", "spr_floweyniceside", "spr_floweypissed",
    "spr_winkstar", "spr_torielflame"
};
using (var w = new TextureWorker()) {
    foreach (var name in want) {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == name);
        if (spr == null) { Console.WriteLine("MISSING " + name); continue; }
        for (int i = 0; i < spr.Textures.Count; i++)
            if (spr.Textures[i]?.Texture != null)
                w.ExportAsPNG(spr.Textures[i].Texture, Path.Combine(dir, $"{name}_{i}.png"), null, false);
        Console.WriteLine($"{name}: {spr.Textures.Count} frames {spr.Width}x{spr.Height} origin {spr.OriginX},{spr.OriginY}");
    }
}
