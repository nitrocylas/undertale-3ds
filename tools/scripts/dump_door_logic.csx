using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return "(none)"; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();
foreach(var ob in new[]{"obj_doorparent","obj_doorA","obj_doorB","obj_doorC","obj_doorD","obj_ruinsdoor1","obj_door_ruins13"}){
  sb.AppendLine("==== "+ob+" : all code events ====");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null && x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){
    sb.AppendLine("  -- "+c.Name.Content+" --"); sb.AppendLine(D(c.Name.Content));
  }
  sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\door_logic.txt",sb.ToString());
Console.WriteLine("ok");
