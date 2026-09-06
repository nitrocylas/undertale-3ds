using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();
foreach(var ob in new[]{"obj_pushrock1","obj_pushrockparent","obj_spikes_room","obj_rock_activator","obj_goofyrock","obj_door_ruins13"}){
  sb.AppendLine("################ "+ob+" ################");
  try{ foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){
    var t=D(c.Name.Content); if(t==null)continue;
    sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t);
  } }catch(Exception e){ sb.AppendLine("(err "+e.Message+")"); }
  sb.AppendLine();
}
// Room data + objects for the rooms after ruins12
bool Skip(string n)=> n==null||n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl"||n.Contains("fakewater")||n=="obj_overworldcontroller";
foreach(var rn in new[]{"room_ruins12A","room_ruins12B","room_ruins13"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null){sb.AppendLine(rn+" MISSING");continue;}
  sb.AppendLine($"==== {rn} {room.Width}x{room.Height}  tiles={room.Tiles.Count} ====");
  foreach(var o in room.GameObjects){ string n=o.ObjectDefinition?.Name?.Content; if(Skip(n))continue; sb.AppendLine($"  {n} @({o.X},{o.Y})"); }
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\mech.txt",sb.ToString());
Console.WriteLine("done");
