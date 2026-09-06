using System; using System.IO; using System.Linq; using System.Text; using System.Collections.Generic;
using UndertaleModLib.Util; using UndertaleModLib.Models;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
string outDir=@"C:\Users\jojoc\undertale-3ds\extracted\slice4"; Directory.CreateDirectory(outDir);
string incDir=@"C:\Users\jojoc\undertale-3ds\include";
var report=new StringBuilder();

// room order 30..50 for target resolution
report.AppendLine("== INTERNAL ORDER 28..48 ==");
for(int i=28;i<=48 && i<Data.Rooms.Count;i++) report.AppendLine($"  {i}: {Data.Rooms[i].Name?.Content}");
report.AppendLine();

(int ox,int oy,int w,int h) Spr(string s){ var sp=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s); if(sp==null)return(0,0,20,20); return((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height);}
(float sx,float sy) RS(string o){switch(o){case "obj_solidlong":return(400f,1f);case "obj_solidlongleft":return(-400f,1f);case "obj_solidtall":return(1f,400f);default:return(1f,1f);}}
bool IsSolid(string n)=>n!=null&&(n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
int ST(string n){switch(n){case "obj_sur":return 1;case "obj_sul":return 2;case "obj_sdr":return 3;case "obj_sdl":return 4;default:return 0;}}

var tilesets=new HashSet<string>();
var interactObjs=new HashSet<string>();
foreach(var rn in new[]{"room_torhouse1","room_torhouse2","room_torhouse3"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null){report.AppendLine(rn+" MISSING");continue;}
  int RW=(int)room.Width,RH=(int)room.Height;
  string dir=Path.Combine(outDir,rn.Replace("room_","")); Directory.CreateDirectory(dir);
  report.AppendLine($"==== {rn} {RW}x{RH} tiles={room.Tiles.Count} ====");
  // tiles
  var tb=new StringBuilder(); tb.AppendLine($"SIZE {RW} {RH}");
  foreach(var t in room.Tiles){ string tsn=t.spriteMode?t.SpriteDefinition?.Name?.Content:t.BackgroundDefinition?.Name?.Content; if(tsn==null)continue; tilesets.Add(tsn); tb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {tsn}"); }
  File.WriteAllText(Path.Combine(dir,"tiles.txt"),tb.ToString());
  // collision header
  var rects=new List<(int,int,int,int,int)>();
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(IsSolid(n)){ string spn=o.ObjectDefinition.Sprite?.Name?.Content; if(spn==null)continue; var(ox,oy,sw,sh)=Spr(spn); var(rsx,rsy)=RS(n); int ty=ST(n);
      float x0=o.X-ox*rsx,y0=o.Y-oy*rsy,x1=x0+sw*rsx,y1=y0+sh*rsy; int rx=(int)Math.Round(Math.Min(x0,x1)),ry=(int)Math.Round(Math.Min(y0,y1)),rw=(int)Math.Round(Math.Abs(x1-x0)),rh=(int)Math.Round(Math.Abs(y1-y0));
      if(ty==0){if(rx<0){rw+=rx;rx=0;}if(ry<0){rh+=ry;ry=0;}if(rx+rw>RW)rw=RW-rx;if(ry+rh>RH)rh=RH-ry;}
      if(rw>0&&rh>0)rects.Add((rx,ry,rw,rh,ty));
    } else {
      report.AppendLine($"  OBJ {n} @({o.X},{o.Y}) spr={o.ObjectDefinition?.Sprite?.Name?.Content}");
      if(n!=null&&(n.Contains("readable")||n.Contains("interact")||n.Contains("door")||n.Contains("toriel")||n.Contains("mirror")||n.Contains("book")||n.Contains("fridge")||n.Contains("plot")||n.Contains("mainchara")||n.Contains("npc")))
        interactObjs.Add(n);
    }
  }
  string macro=rn.Replace("room_","").ToUpper();
  var hb=new StringBuilder();
  hb.AppendLine($"// {rn} collision — Undertale's real solids (room {RW}x{RH}). {{x,y,w,h,type}}.");
  hb.AppendLine($"static const short {macro}_SOLIDS[][5] = {{");
  foreach(var r in rects) hb.AppendLine($"    {{ {r.Item1}, {r.Item2}, {r.Item3}, {r.Item4}, {r.Item5} }},");
  hb.AppendLine("};");
  hb.AppendLine($"#define {macro}_SOLID_COUNT {rects.Count}");
  File.WriteAllText(Path.Combine(incDir,$"solids_{rn.Replace("room_","")}.h"),hb.ToString());
  report.AppendLine($"  -> wrote solids_{rn.Replace("room_","")}.h ({rects.Count} rects)");
  report.AppendLine();
}
// export tilesets
string tsDir=Path.Combine(outDir,"tilesets"); Directory.CreateDirectory(tsDir);
using(var w=new TextureWorker()){
  foreach(var ts in tilesets){ var bg=Data.Backgrounds.FirstOrDefault(b=>b.Name?.Content==ts); if(bg?.Texture!=null) w.ExportAsPNG(bg.Texture,Path.Combine(tsDir,ts+".png"),null,false); }
}
report.AppendLine("tilesets: "+string.Join(",",tilesets));
report.AppendLine();
// decompile interactables
report.AppendLine("################ INTERACTABLE / DOOR / TORIEL CODE ################");
foreach(var ob in interactObjs.OrderBy(x=>x)){
  report.AppendLine("==== "+ob+" ====");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){ var t=D(c.Name.Content); if(t==null)continue; report.AppendLine("-- "+c.Name.Content+" --"); report.AppendLine(t); }
  report.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\report_torhouse_raw.txt",report.ToString());
Console.WriteLine("done tilesets="+tilesets.Count+" interact="+interactObjs.Count);
