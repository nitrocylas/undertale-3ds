using System; using System.IO; using System.Linq; using System.Text;
using UndertaleModLib.Util;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var sb=new StringBuilder();
(int w,int h,int ox,int oy) SP(string s){ var sp=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s); if(sp==null)return(0,0,0,0); return((int)sp.Width,(int)sp.Height,(int)sp.OriginX,(int)sp.OriginY);}
// parents
foreach(var on in new[]{"obj_holedown","obj_holedown2","obj_holeup","obj_cosmetichole"}){
  var o=Data.GameObjects.FirstOrDefault(x=>x.Name?.Content==on);
  sb.AppendLine($"{on}: parent={o?.ParentId?.Name?.Content ?? "-"} sprite={o?.Sprite?.Name?.Content ?? "-"}");
}
sb.AppendLine();
sb.AppendLine("-- obj_cosmetichole code --");
foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_obj_cosmetichole_"))){ var t=D(c.Name.Content); if(t!=null){sb.AppendLine(c.Name.Content); sb.AppendLine(t);} }
sb.AppendLine();
// instances with scale in ruins8 & ruins10
foreach(var rn in new[]{"room_ruins8","room_ruins10"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn);
  sb.AppendLine($"==== {rn} {room.Width}x{room.Height} hole/vent instances ====");
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(n!=null&&(n.Contains("hole")||n.Contains("vent"))){
      string spn=o.ObjectDefinition?.Sprite?.Name?.Content;
      var(w,h,ox,oy)=SP(spn);
      sb.AppendLine($"  {n} @({o.X},{o.Y}) scale({o.ScaleX:0.##},{o.ScaleY:0.##}) spr={spn}[{w}x{h} o{ox},{oy}]");
    }
  }
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\holes_inst.txt",sb.ToString());
Console.WriteLine("done inst");
