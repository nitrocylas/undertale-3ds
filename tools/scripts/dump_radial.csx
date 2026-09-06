using System;
using System.IO;
using System.Linq;
string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\controller";
Directory.CreateDirectory(dir);
var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
foreach (var code in Data.Code) {
    string cn = code.Name?.Content ?? "";
    if (!(cn.Contains("obj_radialfakegen") || cn.Contains("obj_fakepellet"))) continue;
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(dir, cn + ".gml"), t);
        Console.WriteLine("=== " + cn + " ===");
        Console.WriteLine(t);
    } catch (Exception e) { Console.WriteLine("fail " + cn + ": " + e.Message); }
}
