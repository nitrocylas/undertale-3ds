using System;
using System.IO;
using System.Linq;

string outDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice2";
Directory.CreateDirectory(outDir);

// 1. Any string containing the menu keys as substring (would be a JSON blob).
var blobs = Data.Strings
    .Where(s => s.Content != null && s.Content.Contains("instructions_title") && s.Content.Length > 60)
    .Select(s => s.Content).ToList();
File.WriteAllLines(Path.Combine(outDir, "lang_blobs.txt"),
    blobs.Select(b => $"[len {b.Length}] {b.Substring(0, Math.Min(400, b.Length))}"));
Console.WriteLine("JSON-blob candidates: " + blobs.Count);

// 2. Which code entries reference text_data_en (the map builder / loader).
var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
int hits = 0;
foreach (var code in Data.Code)
{
    string cn = code.Name?.Content ?? "";
    // Only decompile root scripts to keep it fast; skip child gml_Object structs.
    if (!cn.StartsWith("gml_Script_") && !cn.Contains("_init") && !cn.Contains("84")) continue;
    string t;
    try { t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString(); }
    catch { continue; }
    if (t.Contains("text_data_en"))
    {
        File.WriteAllText(Path.Combine(outDir, "builder_" + cn + ".gml"), t);
        Console.WriteLine("builder: " + cn + " (" + t.Length + " chars)");
        hits++;
    }
}
Console.WriteLine("builders found: " + hits);
