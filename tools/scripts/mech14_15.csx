using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var dset = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,dset).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();
foreach(var ob in new[]{"obj_redswitch_1","obj_blueswitch_1","obj_greenswitch_1","obj_colorswitchparent","obj_ribbon_pickup","obj_plotswitch3","obj_holedown3","obj_holeup2","obj_napstablook2","obj_door_ruins13"}){
  sb.AppendLine("#### "+ob+" ####");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null && x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){
    var t=D(c.Name.Content); if(t==null)continue; sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t);
  }
  sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\mech14_15.txt", sb.ToString());
Console.WriteLine("done");
