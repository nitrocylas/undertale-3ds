using System; using System.IO; using System.Linq; using System.Text;
using System.Collections.Generic;
using UndertaleModLib.Util;
string root=@"C:\Users\jojoc\undertale-3ds";
string[] rooms={"room_ruins16","room_ruins17","room_ruins18OLD","room_ruins19"};

// --- internal room order context ---
var sb=new StringBuilder();
sb.AppendLine("== INTERNAL ROOM ORDER (index : name) ==");
for(int i=0;i<Data.Rooms.Count;i++){ string nm=Data.Rooms[i].Name?.Content??"?";
  if(nm.Contains("ruins")||nm.Contains("torhouse")) sb.AppendLine($"  {i} : {nm}"); }
sb.AppendLine();

(int ox,int oy,int w,int h) Spr(string s){ var sp=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s); if(sp==null)return(0,0,20,20); return((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height);}
(float sx,float sy) RS(string o){switch(o){case "obj_solidlong":return(400f,1f);case "obj_solidlongleft":return(-400f,1f);case "obj_solidtall":return(1f,400f);default:return(1f,1f);}}
bool IsSolid(string n)=>n!=null&&(n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
int ST(string n){switch(n){case "obj_sur":return 1;case "obj_sul":return 2;case "obj_sdr":return 3;case "obj_sdl":return 4;default:return 0;}}
bool IsDoor(string n)=>n!=null&&(n.Contains("door")||n.Contains("warp"));

var allTs=new HashSet<string>();
foreach(var rn in rooms){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn);
  if(room==null){sb.AppendLine($"==== {rn} : NOT FOUND ====");continue;}
  int RW=(int)room.Width,RH=(int)room.Height;
  sb.AppendLine($"==== {rn} : {RW}x{RH}  tiles={room.Tiles.Count} ====");
  // objects
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(IsSolid(n)) continue;
    string tag = (IsDoor(n)||n=="obj_mainchara")?"* ":"  ";
    sb.AppendLine($"{tag}{n} @({o.X},{o.Y}) spr={o.ObjectDefinition?.Sprite?.Name?.Content}");
  }
  // solids header
  var rects=new List<(int,int,int,int,int)>();
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content; if(!IsSolid(n))continue;
    string spn=o.ObjectDefinition.Sprite?.Name?.Content; if(spn==null)continue;
    var(ox,oy,sw,sh)=Spr(spn); var(rsx,rsy)=RS(n); int ty=ST(n);
    float x0=o.X-ox*rsx,y0=o.Y-oy*rsy,x1=x0+sw*rsx,y1=y0+sh*rsy;
    int rx=(int)Math.Round(Math.Min(x0,x1)),ry=(int)Math.Round(Math.Min(y0,y1)),rw=(int)Math.Round(Math.Abs(x1-x0)),rh=(int)Math.Round(Math.Abs(y1-y0));
    if(ty==0){if(rx<0){rw+=rx;rx=0;}if(ry<0){rh+=ry;ry=0;}if(rx+rw>RW)rw=RW-rx;if(ry+rh>RH)rh=RH-ry;}
    if(rw>0&&rh>0)rects.Add((rx,ry,rw,rh,ty));
  }
  string macro=rn.Replace("room_","").ToUpper();
  var hb=new StringBuilder();
  hb.AppendLine($"// {rn} collision — Undertale's real solids (room {RW}x{RH}). {{x,y,w,h,type}}.");
  hb.AppendLine($"static const short {macro}_SOLIDS[][5] = {{");
  foreach(var r in rects) hb.AppendLine($"    {{ {r.Item1}, {r.Item2}, {r.Item3}, {r.Item4}, {r.Item5} }},");
  hb.AppendLine("};");
  hb.AppendLine($"#define {macro}_SOLID_COUNT {rects.Count}");
  File.WriteAllText(Path.Combine(root,"include",$"solids_{rn.Replace("room_","")}.h"), hb.ToString());
  sb.AppendLine($"  -> wrote solids_{rn.Replace("room_","")}.h ({rects.Count} rects)");
  // tiles export
  string dir=Path.Combine(root,"extracted","slice4",rn.Replace("room_",""));
  Directory.CreateDirectory(dir);
  var tb=new StringBuilder(); tb.AppendLine($"SIZE {RW} {RH}");
  foreach(var t in room.Tiles){ string ts=t.spriteMode?t.SpriteDefinition?.Name?.Content:t.BackgroundDefinition?.Name?.Content; if(ts==null)continue; allTs.Add(ts);
    tb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {ts}"); }
  File.WriteAllText(Path.Combine(dir,"tiles.txt"), tb.ToString());
  sb.AppendLine();
}
// tilesets
string tsDir=Path.Combine(root,"extracted","slice4","tilesets"); Directory.CreateDirectory(tsDir);
using(var w=new TextureWorker()){ foreach(var ts in allTs){ var bg=Data.Backgrounds.FirstOrDefault(b=>b.Name?.Content==ts); if(bg?.Texture!=null) w.ExportAsPNG(bg.Texture, Path.Combine(tsDir,ts+".png"), null, false); } }
sb.AppendLine("tilesets: "+string.Join(",",allTs));
File.WriteAllText(Path.Combine(root,"extracted","ruins16_19_struct.txt"), sb.ToString());
Console.WriteLine("STRUCT DONE");
