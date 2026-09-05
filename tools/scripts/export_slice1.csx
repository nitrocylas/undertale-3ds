// Slice 1 (intro storyboard) export ONLY. Pulls the intro sprites as PNG frames and the
// decompiled code that drives the intro, so we can rebuild it faithfully on the 3DS.
using System;
using System.IO;
using System.Linq;
using UndertaleModLib.Util;
using UndertaleModLib.Models;

string root   = @"C:\Users\jojoc\undertale-3ds\extracted\slice1";
string sprDir = Path.Combine(root, "sprites");
string codeDir= Path.Combine(root, "code");
Directory.CreateDirectory(sprDir);
Directory.CreateDirectory(codeDir);

// --- 1. Export specific sprites (all frames) ---
string[] wantSprites = { "spr_introimage", "spr_introlast", "spr_heart" };
using (var worker = new TextureWorker())
{
    foreach (var name in wantSprites)
    {
        var spr = Data.Sprites.FirstOrDefault(s => s.Name?.Content == name);
        if (spr == null) { Console.WriteLine("MISSING sprite: " + name); continue; }
        for (int i = 0; i < spr.Textures.Count; i++)
        {
            var pageItem = spr.Textures[i]?.Texture;
            if (pageItem == null) continue;
            worker.ExportAsPNG(pageItem, Path.Combine(sprDir, $"{name}_{i}.png"), null, false);
        }
        Console.WriteLine($"sprite {name}: {spr.Textures.Count} frame(s), {spr.Width}x{spr.Height}");
    }
}

// --- 2. Decompile the intro-driving code ---
var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
string[] objMatch = { "obj_introimage", "obj_introfader", "obj_introlast", "obj_intromenu" };
int dumped = 0;
foreach (var code in Data.Code)
{
    string cn = code.Name?.Content ?? "";
    if (!objMatch.Any(m => cn.Contains(m))) continue;
    try
    {
        string text = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(codeDir, cn + ".gml"), text);
        dumped++;
    }
    catch (Exception e) { Console.WriteLine("decompile fail " + cn + ": " + e.Message); }
}
Console.WriteLine("SLICE1 EXPORT DONE. code entries dumped=" + dumped);
