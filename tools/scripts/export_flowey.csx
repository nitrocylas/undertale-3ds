// Decompile Flowey's overworld intro scene + export his sprites and the pellet.
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;

string codeDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\code";
string sprDir  = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\sprites";
Directory.CreateDirectory(codeDir);
Directory.CreateDirectory(sprDir);

var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
foreach (var code in Data.Code) {
    string cn = code.Name?.Content ?? "";
    if (!(cn.Contains("obj_floweytalker1") || cn.Contains("obj_friendlypellet") || cn.Contains("obj_flowey_friendscene")))
        continue;
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(codeDir, cn + ".gml"), t);
        Console.WriteLine("code " + cn);
    } catch (Exception e) { Console.WriteLine("fail " + cn + ": " + e.Message); }
}

using (var worker = new TextureWorker()) {
    string[] want = { "spr_flowey", "spr_flowey_riseanim", "spr_floweynice", "spr_floweyniceside",
                      "spr_floweyevil", "spr_floweypissed", "spr_floweygrin", "spr_flowey_empty",
                      "obj_friendlypellet", "spr_friendlypellet" };
    foreach (var name in want) {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == name);
        if (spr == null) continue;
        for (int i = 0; i < spr.Textures.Count; i++)
            if (spr.Textures[i]?.Texture != null)
                worker.ExportAsPNG(spr.Textures[i].Texture, Path.Combine(sprDir, $"{name}_{i}.png"), null, false);
        Console.WriteLine($"spr {name}: {spr.Textures.Count} frames {spr.Width}x{spr.Height}");
    }
    // The pellet sprite is whatever obj_friendlypellet uses.
    var pel = Data.GameObjects.FirstOrDefault(g => g.Name?.Content == "obj_friendlypellet");
    Console.WriteLine("pellet sprite = " + (pel?.Sprite?.Name?.Content ?? "none"));
}
