// Extract Toriel's overworld sprites + a dialogue face, and locate her arrival dialogue.
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;

string sprDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice4\toriel\sprites";
string codeDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice4\toriel\code";
Directory.CreateDirectory(sprDir);
Directory.CreateDirectory(codeDir);

using (var w = new TextureWorker()) {
    string[] want = { "spr_toriel_d", "spr_toriel_u", "spr_toriel_l", "spr_toriel_r",
                      "spr_toriel_dt", "spr_toriel_ut", "spr_toriel_lt", "spr_toriel_rt",
                      "spr_face_torielblink", "spr_face_toriel" };
    foreach (var name in want) {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == name);
        if (spr == null) { Console.WriteLine("MISSING " + name); continue; }
        for (int i = 0; i < spr.Textures.Count; i++)
            if (spr.Textures[i]?.Texture != null)
                w.ExportAsPNG(spr.Textures[i].Texture, Path.Combine(sprDir, $"{name}_{i}.png"), null, false);
        Console.WriteLine($"{name}: {spr.Textures.Count} frames {spr.Width}x{spr.Height}");
    }
}

// Decompile the cutscene controller (Toriel's arrival + leading you).
var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
foreach (var code in Data.Code) {
    string cn = code.Name?.Content ?? "";
    if (!cn.Contains("obj_torielcutscene")) continue;
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(codeDir, cn + ".gml"), t);
        Console.WriteLine("code " + cn + " (" + t.Length + ")");
    } catch (Exception e) { Console.WriteLine("fail " + cn + ": " + e.Message); }
}
