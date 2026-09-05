// Extract Flowey's wink face, the twinkle star, and the battle-start sound.
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;

string sprDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\sprites";
string sndDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\snd";
Directory.CreateDirectory(sprDir);
Directory.CreateDirectory(sndDir);

using (var worker = new TextureWorker()) {
    foreach (var name in new[] { "spr_floweywink", "spr_glowstar", "spr_regstar" }) {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == name);
        if (spr == null) { Console.WriteLine("MISSING " + name); continue; }
        for (int i = 0; i < spr.Textures.Count; i++)
            if (spr.Textures[i]?.Texture != null)
                worker.ExportAsPNG(spr.Textures[i].Texture, Path.Combine(sprDir, $"{name}_{i}.png"), null, false);
        Console.WriteLine($"{name}: {spr.Textures.Count} frames {spr.Width}x{spr.Height}");
    }
}

// Export embedded sounds. Undertale stores them as embedded audio (WAV/OGG) referenced by
// UndertaleSound entries; write the raw embedded data to a file.
foreach (var name in new[] { "snd_heartshot", "snd_damage" }) {
    var snd = Data.Sounds.FirstOrDefault(s => s.Name?.Content == name);
    if (snd == null) { Console.WriteLine("MISSING snd " + name); continue; }
    var ext = "bin";
    byte[] data = null;
    if (snd.AudioFile != null && snd.AudioFile.Data != null) { data = snd.AudioFile.Data; ext = "wav"; }
    if (data != null) {
        File.WriteAllBytes(Path.Combine(sndDir, name + "." + ext), data);
        Console.WriteLine($"{name}: {data.Length} bytes -> {ext}  (file='{snd.File?.Content}')");
    } else {
        Console.WriteLine($"{name}: no embedded data; external file='{snd.File?.Content}' flags={snd.Flags}");
    }
}
