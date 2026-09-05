// Pull the intro narration. In this build scr_gettext looks up a ds_map built from string
// pairs "key","value". We find every string equal to a key obj_introimage_70..85 and the
// following value string, plus dump scr_gettext so we understand the lookup.
using System;
using System.IO;
using System.Linq;

string root = @"C:\Users\jojoc\undertale-3ds\extracted\slice1";
Directory.CreateDirectory(root);

// Decompile scr_gettext + scr_84_init (text tables) if present.
var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
foreach (var wanted in new[] { "scr_gettext", "scr_84_init", "scr_langswap" })
{
    var code = Data.Code.FirstOrDefault(c => c.Name?.Content == "gml_Script_" + wanted);
    if (code == null) continue;
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(root, wanted + ".gml"), t);
        Console.WriteLine("dumped " + wanted);
    } catch (Exception e) { Console.WriteLine("fail " + wanted + ": " + e.Message); }
}

// Dump ALL strings, indexed, so we can locate the intro narration and any control codes.
var all = Data.Strings.Select((s, i) => $"[{i}] {s.Content?.Replace("\n", "\\n")}").ToList();
File.WriteAllLines(Path.Combine(root, "all_strings.txt"), all);
Console.WriteLine("total strings: " + Data.Strings.Count);
