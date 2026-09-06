using System; using System.IO; using System.Linq; using System.Text; using System.Collections.Generic;
using UndertaleModLib.Util; using UndertaleModLib.Models;
var gctx = new GlobalDecompileContext(Data);
var dset = Data.ToolInfo.DecompilerSettings;
string Dec(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,dset).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
string baseDir=@"C:\Users\jojoc\undertale-3ds";
string slice4=Path.Combine(baseDir,@"extracted\slice4");
string incDir=Path.Combine(baseDir,"include");
var report=new StringBuilder();
var tilesets=new HashSet<string>();
(int ox,int oy,int w,int h) Spr(string s){ var sp=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s); if(sp==null)return(0,0,20,20); return((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height); }
(float,float) RS(string o){switch(o){case "obj_solidlong":return(400f,1f);case "obj_solidlongleft":return(-400f,1f);case "obj_solidtall":return(1f,400f);default:return(1f,1f);}}
bool IsSolid(string n)=>n!=null&&(n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
int ST(string n){switch(n){case "obj_sur":return 1;case "obj_sul":return 2;case "obj_sdr":return 3;case "obj_sdl":return 4;default:return 0;}}

foreach(var rn in new[]{"room_ruins12A","room_ruins12B","room_ruins13"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn);
  if(room==null){report.AppendLine(rn+" MISSING");continue;}
  int RW=(int)room.Width,RH=(int)room.Height;
  report.AppendLine($"==== {rn} {RW}x{RH} tiles={room.Tiles.Count} ====");
  string dir=Path.Combine(slice4, rn.Replace("room_","")); Directory.CreateDirectory(dir);
  var tb=new StringBuilder(); tb.AppendLine($"SIZE {RW} {RH}");
  foreach(var t in room.Tiles){ string ts=t.spriteMode?t.SpriteDefinition?.Name?.Content:t.BackgroundDefinition?.Name?.Content; if(ts==null)continue; tilesets.Add(ts); tb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {ts}"); }
  File.WriteAllText(Path.Combine(dir,"tiles.txt"),tb.ToString());
  var rects=new List<(int,int,int,int,int)>();
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(IsSolid(n)){ string spn=o.ObjectDefinition.Sprite?.Name?.Content; if(spn==null)continue; var(ox,oy,sw,sh)=Spr(spn); var(rsx,rsy)=RS(n); int ty=ST(n);
      float x0=o.X-ox*rsx,y0=o.Y-oy*rsy,x1=x0+sw*rsx,y1=y0+sh*rsy; int rx=(int)Math.Round(Math.Min(x0,x1)),ry=(int)Math.Round(Math.Min(y0,y1)),rw=(int)Math.Round(Math.Abs(x1-x0)),rh=(int)Math.Round(Math.Abs(y1-y0));
      if(ty==0){if(rx<0){rw+=rx;rx=0;}if(ry<0){rh+=ry;ry=0;}if(rx+rw>RW)rw=RW-rx;if(ry+rh>RH)rh=RH-ry;}
      if(rw>0&&rh>0)rects.Add((rx,ry,rw,rh,ty)); }
    else report.AppendLine($"  OBJ {n} @({o.X},{o.Y}) spr={o.ObjectDefinition?.Sprite?.Name?.Content}");
  }
  string macro=rn.Replace("room_","").ToUpper();
  var hb=new StringBuilder();
  hb.AppendLine($"// {rn} collision — Undertale's real solids (room {RW}x{RH}). {{x,y,w,h,type}}.");
  hb.AppendLine($"static const short {macro}_SOLIDS[][5] = {{");
  foreach(var r in rects)hb.AppendLine($"    {{ {r.Item1}, {r.Item2}, {r.Item3}, {r.Item4}, {r.Item5} }},");
  hb.AppendLine("};"); hb.AppendLine($"#define {macro}_SOLID_COUNT {rects.Count}");
  File.WriteAllText(Path.Combine(incDir,$"solids_{rn.Replace("room_","")}.h"),hb.ToString());
  report.AppendLine($"  -> solids_{rn.Replace("room_","")}.h ({rects.Count} rects)");
}
string tsDir=Path.Combine(slice4,"tilesets"); Directory.CreateDirectory(tsDir);
using(var w=new TextureWorker()){ foreach(var ts in tilesets){ var bg=Data.Backgrounds.FirstOrDefault(b=>b.Name?.Content==ts); if(bg?.Texture!=null)w.ExportAsPNG(bg.Texture,Path.Combine(tsDir,ts+".png"),null,false); } }
report.AppendLine("tilesets: "+string.Join(",",tilesets));

report.AppendLine("\n#### obj_napstablook1 ####");
foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_obj_napstablook1_"))){ var t=Dec(c.Name.Content); if(t==null)continue; report.AppendLine("-- "+c.Name.Content+" --"); report.AppendLine(t); }
using(var w=new TextureWorker()){ var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content=="spr_napstablook_gr"); if(s?.Textures?.Count>0&&s.Textures[0]?.Texture!=null){ w.ExportAsPNG(s.Textures[0].Texture,Path.Combine(baseDir,@"extracted\napstablook.png"),null,true); report.AppendLine($"spr_napstablook_gr {s.Width}x{s.Height} origin({s.OriginX},{s.OriginY}) exported"); } }

report.AppendLine("\n#### spider/web/shop objects ####");
foreach(var o in Data.GameObjects){ var n=o.Name?.Content; if(n!=null&&(n.Contains("spider")||n.Contains("web")||n.Contains("bakesale")||n.Contains("muffet"))) report.AppendLine("OBJ "+n+" spr="+o.Sprite?.Name?.Content); }
foreach(var s in Data.Sprites){ var n=s.Name?.Content; if(n!=null&&(n.Contains("spider")||n.Contains("web"))) report.AppendLine($"SPR {n} {s.Width}x{s.Height}"); }
foreach(var room in Data.Rooms){ foreach(var o in room.GameObjects){ var n=o.ObjectDefinition?.Name?.Content; if(n!=null&&(n.Contains("spider")||n.Contains("bakesale"))) report.AppendLine($"ROOM {room.Name?.Content}: {n} @({o.X},{o.Y})"); } }
File.WriteAllText(Path.Combine(baseDir,@"extracted\fork_recon.txt"),report.ToString());
Console.WriteLine("recon done");
