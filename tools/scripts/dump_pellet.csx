using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();
foreach(var ob in new[]{"obj_friendlypellet","obj_fakeheart","obj_heart","obj_flowey_master"}){
  sb.AppendLine("################ "+ob+" ################");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_")).OrderBy(x=>x.Name.Content)){
    var t=D(c.Name.Content); if(t==null)continue;
    sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t);
  }
  sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\pellet_mech.txt",sb.ToString());
Console.WriteLine("done");
