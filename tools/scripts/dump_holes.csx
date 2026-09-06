using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();

// internal room order (for room_next/previous/index resolution)
sb.AppendLine("== ROOM ORDER (ruins) ==");
for(int i=0;i<Data.Rooms.Count;i++){ string nm=Data.Rooms[i].Name?.Content??"?"; if(nm.Contains("ruins")||nm.Contains("home")||nm.Contains("torhouse")) sb.AppendLine($"  {i}: {nm}"); }
sb.AppendLine();

// hole/vent object code
foreach(var ob in new[]{"obj_holedown","obj_holedown2","obj_holeup","obj_holedownparent","obj_holemask","obj_ruinspit","obj_ruinspitfall"}){
  var codes=Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_")).ToList();
  if(codes.Count==0) continue;
  sb.AppendLine("################ "+ob+" ################");
  foreach(var c in codes){ var t=D(c.Name.Content); if(t==null)continue; sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t); }
  sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\holes_logic.txt",sb.ToString());
Console.WriteLine("done logic");
