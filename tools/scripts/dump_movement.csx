// Decompile the player's movement + collision so we can replicate it exactly.
using System;
using System.IO;
using System.Linq;
string dir = @"C:\Users\jojoc\undertale-3ds\extracted\movement";
Directory.CreateDirectory(dir);
var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
string[] objs = { "obj_mainchara", "obj_solidsmall", "obj_solidtall", "obj_solidlong",
                  "obj_sur", "obj_sul", "obj_sdr", "obj_sdl" };
foreach (var code in Data.Code) {
    string cn = code.Name?.Content ?? "";
    if (!objs.Any(o => cn.Contains(o))) continue;
    if (cn.Contains("Draw")) continue; // skip draw events
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(dir, cn + ".gml"), t);
        Console.WriteLine(cn + " (" + t.Length + ")");
    } catch (Exception e) { Console.WriteLine("fail " + cn + ": " + e.Message); }
}
// Also the solid objects' collision-mask sprites and whether they're "solid".
foreach (var n in objs) {
    var o = Data.GameObjects.FirstOrDefault(g => g.Name?.Content == n);
    if (o != null)
        Console.WriteLine($"OBJ {n}: sprite={o.Sprite?.Name?.Content} mask={o.TextureMaskId?.Name?.Content} solid={o.Solid} shape={o.CollisionShape}");
}
