using System; using System.IO; using System.Linq; using System.Text;
using UndertaleModLib.Util;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();

// 1) Mechanic object code
foreach(var ob in new[]{"obj_pushrock1","obj_pushrock2","obj_pushrock3","obj_pushrockparent","obj_spikes_room","obj_rock_activator","obj_goofyrock"}){
  sb.AppendLine("################ "+ob+" ################");
  var codes=Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_")).ToList();
  if(codes.Count==0) sb.AppendLine("(no code events)");
  foreach(var c in codes){ var t=D(c.Name.Content); sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t); }
  sb.AppendLine();
}

// 2) msc 505 (goofyrock) branch from dialoguer SCR_TEXT script
sb.AppendLine("################ SCR_TEXT case 505 (goofyrock) ################");
var scr=D("gml_Script_SCR_TEXT");
if(scr!=null){
  int idx=scr.IndexOf("case 505:");
  if(idx>=0){ int end=scr.IndexOf("case 506:", idx); if(end<0)end=Math.Min(scr.Length,idx+2500); sb.AppendLine(scr.Substring(idx, end-idx)); }
  else sb.AppendLine("(case 505 not found in SCR_TEXT)");
}

// 3) Sprite info + padded PNG export
string outDir=@"C:\Users\jojoc\undertale-3ds\extracted\rockassets"; Directory.CreateDirectory(outDir);
sb.AppendLine(); sb.AppendLine("################ SPRITES ################");
using(var w=new TextureWorker()){
  foreach(var sn in new[]{"spr_rock","spr_spiketile","spr_spikes_room"}){
    var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==sn); if(s==null){sb.AppendLine(sn+": MISSING");continue;}
    sb.AppendLine($"{sn}: {s.Width}x{s.Height} origin({s.OriginX},{s.OriginY}) frames={s.Textures.Count}");
    for(int i=0;i<s.Textures.Count;i++) if(s.Textures[i]?.Texture!=null) w.ExportAsPNG(s.Textures[i].Texture, Path.Combine(outDir,$"{sn}_{i}.png"), null, true);
  }
}

// 4) Verify positions in ruins9 & ruins11
sb.AppendLine(); sb.AppendLine("################ POSITIONS ################");
foreach(var rn in new[]{"room_ruins9","room_ruins11"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null)continue;
  sb.AppendLine($"== {rn} {room.Width}x{room.Height} ==");
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content; if(n==null)continue;
    if(n.Contains("rock")||n.Contains("spike")||n.Contains("activator")||n.Contains("plotwall")||n.Contains("marker"))
      sb.AppendLine($"  {n} @({o.X},{o.Y}) scale({o.ScaleX},{o.ScaleY})");
  }
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\rock_dump.txt",sb.ToString());
Console.WriteLine("done rock_dump.txt");
