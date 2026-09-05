// Slice 2 (main menu) research: decompile the naming/continue-menu scripts.
using System;
using System.IO;
using System.Linq;

string codeDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice2\code";
Directory.CreateDirectory(codeDir);

var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
string[] wanted = {
    "gml_Script_scr_namingscreen",
    "gml_Script_scr_namingscreen_setup",
    "gml_Script_scr_namingscreen_check",
    "gml_Script_scr_drawtext_centered",
    "gml_Script_scr_setfont",
};
foreach (var name in wanted)
{
    var code = Data.Code.FirstOrDefault(c => c.Name?.Content == name);
    if (code == null) { Console.WriteLine("MISSING " + name); continue; }
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(codeDir, name + ".gml"), t);
        Console.WriteLine("dumped " + name + " (" + t.Length + " chars)");
    } catch (Exception e) { Console.WriteLine("fail " + name + ": " + e.Message); }
}
