using System; using System.IO; using System.Linq; using System.Text;
using UndertaleModLib.Util;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch{return null;}}
var sb=new StringBuilder();
// Find any code that references flag[34] or the candy text keys or msc 508 candy handling.
string[] needles = { "flag[34]", "1624", "1628", "1631", "1625", "candydish", "spr_candydish" };
foreach(var c in Data.Code){
  var n=c.Name?.Content; if(n==null) continue;
  if(!n.StartsWith("gml_Object_")&&!n.StartsWith("gml_Script_")&&!n.StartsWith("gml_GlobalScript_")) continue;
  var t=D(n); if(t==null) continue;
  if(needles.Any(k=>t.Contains(k))){
    sb.AppendLine("################ "+n+" ################");
    sb.AppendLine(t); sb.AppendLine();
  }
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\candy_logic.txt", sb.ToString());
// Export all frames of the three candy sprites, padded.
string outDir=@"C:\Users\jojoc\undertale-3ds\extracted\candy"; Directory.CreateDirectory(outDir);
using(var w=new TextureWorker()){
  foreach(var sn in new[]{"spr_candydish","spr_candydish_bad","spr_candydish2"}){
    var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==sn); if(s==null) continue;
    for(int i=0;i<s.Textures.Count;i++) if(s.Textures[i]?.Texture!=null) w.ExportAsPNG(s.Textures[i].Texture, Path.Combine(outDir,$"{sn}_{i}.png"), null, true);
  }
}
Console.WriteLine("done");
