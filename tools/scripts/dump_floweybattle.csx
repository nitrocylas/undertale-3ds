// Decompile the full Flowey intro battle controller so we can replicate it 1:1.
using System;
using System.IO;
using System.Linq;

string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\flowey\controller";
Directory.CreateDirectory(dir);

var ctx = new GlobalDecompileContext(Data);
var settings = Data.ToolInfo.DecompilerSettings;
string[] objs = { "obj_floweybattle1", "obj_floweybattler2", "obj_flowey_master",
                  "obj_blconwdflowey", "obj_floweybodyparent" };
int n = 0;
foreach (var code in Data.Code) {
    string cn = code.Name?.Content ?? "";
    if (!objs.Any(o => cn.Contains(o))) continue;
    try {
        string t = new Underanalyzer.Decompiler.DecompileContext(ctx, code, settings).DecompileToString();
        File.WriteAllText(Path.Combine(dir, cn + ".gml"), t);
        Console.WriteLine(cn + " (" + t.Length + " chars)");
        n++;
    } catch (Exception e) { Console.WriteLine("fail " + cn + ": " + e.Message); }
}
Console.WriteLine("dumped " + n + " code entries");
