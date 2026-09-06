using System; using System.IO; using System.Linq; using System.Text;
using System.Collections.Generic;
using UndertaleModLib.Util;
using UndertaleModLib.Models;

string outInc = @"C:\Users\jojoc\undertale-3ds\include";
string baseDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice4";
string rep = @"C:\Users\jojoc\undertale-3ds\extracted\ruins14_15_raw.txt";
string[] rooms = { "room_ruins14","room_ruins15A","room_ruins15B","room_ruins15C","room_ruins15D","room_ruins15E" };

var gctx = new GlobalDecompileContext(Data);
var dset = Data.ToolInfo.DecompilerSettings;
string DD(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,dset).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}

(int ox,int oy,int w,int h) Spr(string s){ var sp=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s); if(sp==null)return(0,0,20,20); return((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height); }
(float sx,float sy) RScale(string o){ switch(o){ case "obj_solidlong":return(400f,1f); case "obj_solidlongleft":return(-400f,1f); case "obj_solidtall":return(1f,400f); default:return(1f,1f);} }
bool IsSolid(string n)=> n!=null && (n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
int SlopeType(string n){ switch(n){ case "obj_sur":return 1; case "obj_sul":return 2; case "obj_sdr":return 3; case "obj_sdl":return 4; default:return 0; } }

var sb = new StringBuilder();
var allTilesets = new HashSet<string>();
var interactObjs = new HashSet<string>();

// room order for context
sb.AppendLine("== ROOM ORDER ==");
for(int i=0;i<Data.Rooms.Count;i++){ string nm=Data.Rooms[i].Name?.Content??"?"; if(nm.Contains("ruins")||nm.Contains("torhouse")) sb.AppendLine($"  {i}: {nm}"); }
sb.AppendLine();

foreach(var rn in rooms){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn);
  if(room==null){ sb.AppendLine($"== {rn} MISSING =="); continue; }
  int RW=(int)room.Width, RH=(int)room.Height;
  sb.AppendLine($"== {rn}  {RW}x{RH}  tiles={room.Tiles.Count} ==");
  string dir=Path.Combine(baseDir, rn.Replace("room_",""));
  Directory.CreateDirectory(dir);

  // tiles.txt
  var tb=new StringBuilder(); tb.AppendLine($"SIZE {RW} {RH}");
  foreach(var t in room.Tiles){ string ts=t.spriteMode?t.SpriteDefinition?.Name?.Content:t.BackgroundDefinition?.Name?.Content; if(ts==null)continue; allTilesets.Add(ts); tb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {ts}"); }
  File.WriteAllText(Path.Combine(dir,"tiles.txt"), tb.ToString());

  // collision header
  var rects=new List<(int,int,int,int,int)>();
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(!IsSolid(n)) continue;
    string spn=o.ObjectDefinition.Sprite?.Name?.Content; if(spn==null)continue;
    var(ox,oy,sw,sh)=Spr(spn); var(rsx,rsy)=RScale(n); int type=SlopeType(n);
    float x0=o.X-ox*rsx,y0=o.Y-oy*rsy,x1=x0+sw*rsx,y1=y0+sh*rsy;
    int rx=(int)Math.Round(Math.Min(x0,x1)),ry=(int)Math.Round(Math.Min(y0,y1)),rw=(int)Math.Round(Math.Abs(x1-x0)),rh=(int)Math.Round(Math.Abs(y1-y0));
    if(type==0){ if(rx<0){rw+=rx;rx=0;} if(ry<0){rh+=ry;ry=0;} if(rx+rw>RW)rw=RW-rx; if(ry+rh>RH)rh=RH-ry; }
    if(rw>0&&rh>0) rects.Add((rx,ry,rw,rh,type));
  }
  string macro=rn.Replace("room_","").ToUpper();
  var hb=new StringBuilder();
  hb.AppendLine($"// {rn} collision — Undertale's real solids (room {RW}x{RH}). {{x,y,w,h,type}}; 0=rect,1-4=slopes.");
  hb.AppendLine($"static const short {macro}_SOLIDS[][5] = {{");
  foreach(var r in rects) hb.AppendLine($"    {{ {r.Item1}, {r.Item2}, {r.Item3}, {r.Item4}, {r.Item5} }},");
  hb.AppendLine("};");
  hb.AppendLine($"#define {macro}_SOLID_COUNT {rects.Count}");
  File.WriteAllText(Path.Combine(outInc,$"solids_{rn.Replace("room_","")}.h"), hb.ToString());
  sb.AppendLine($"  solids: {rects.Count} rects");

  // objects (non-solid): doors, mainchara, NPCs, signs, interactables
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(n==null||IsSolid(n)||n=="obj_overworldcontroller"||n.Contains("fakewater")) continue;
    sb.AppendLine($"  OBJ {n} @({o.X},{o.Y}) spr={o.ObjectDefinition?.Sprite?.Name?.Content}");
    if(n.Contains("readable")||n.Contains("sign")||n.Contains("interactable")||n.Contains("plaque")||n.Contains("froggit")||n.Contains("npc")||n.Contains("dummy")||n.Contains("vending")) interactObjs.Add(n);
  }
  sb.AppendLine();
}

// decompile the interactable objects' event code (to find msc / text keys)
sb.AppendLine("== INTERACTABLE OBJECT CODE ==");
foreach(var ob in interactObjs.OrderBy(x=>x)){
  sb.AppendLine("#### "+ob+" ####");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null && x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){
    var t=DD(c.Name.Content); if(t==null)continue;
    sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t);
  }
  sb.AppendLine();
}

// export tilesets
string tsDir=Path.Combine(baseDir,"tilesets"); Directory.CreateDirectory(tsDir);
using(var w=new TextureWorker()){
  foreach(var ts in allTilesets){ var bg=Data.Backgrounds.FirstOrDefault(b=>b.Name?.Content==ts); if(bg?.Texture!=null) w.ExportAsPNG(bg.Texture, Path.Combine(tsDir,ts+".png"), null, false); }
}
sb.AppendLine("tilesets: "+string.Join(",", allTilesets));
File.WriteAllText(rep, sb.ToString());
Console.WriteLine("DONE tilesets="+allTilesets.Count);
