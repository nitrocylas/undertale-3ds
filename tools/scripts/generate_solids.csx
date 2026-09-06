// Generate accurate collision headers for the Ruins from Undertale's OWN room data.
// Solid rects come straight from each instance's sprite box x the runtime scale
// (solidlong xscale=400, solidtall yscale=400 per their Create events). Long/tall walls are
// clamped to the room; small/wide/tall_2 blocks and the 20x20 slope triangles pass through.
// type: 0=rect, 1=sur, 2=sul, 3=sdr, 4=sdl.
using System;
using System.IO;
using System.Linq;
using System.Text;

string[] rooms = { "room_ruins1","room_ruins2","room_ruins3","room_ruins4","room_ruins5",
                   "room_ruins6","room_ruins7","room_ruins8","room_ruins9","room_ruins10",
                   "room_ruins11","room_ruins12" };
string outDir = @"C:\Users\jojoc\undertale-3ds\include";

(int ox,int oy,int w,int h) Spr(string s){
    var sp = Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s);
    if(sp==null) return (0,0,20,20);
    return ((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height);
}
(float sx,float sy) RScale(string o){
    switch(o){ case "obj_solidlong": return (400f,1f);
               case "obj_solidlongleft": return (-400f,1f);
               case "obj_solidtall": return (1f,400f);
               default: return (1f,1f); }
}
bool IsSolid(string n)=> n!=null && (n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
int SlopeType(string n){ switch(n){ case "obj_sur":return 1; case "obj_sul":return 2;
                                     case "obj_sdr":return 3; case "obj_sdl":return 4; default:return 0; } }
var dims = new StringBuilder();
foreach(var rn in rooms){
    var room = Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn);
    if(room==null) continue;
    int RW=(int)room.Width, RH=(int)room.Height;
    dims.AppendLine($"{rn}: {RW}x{RH}");
    var rects = new System.Collections.Generic.List<(int,int,int,int,int)>();
    foreach(var o in room.GameObjects){
        string n=o.ObjectDefinition?.Name?.Content;
        if(!IsSolid(n)) continue;
        string spn=o.ObjectDefinition.Sprite?.Name?.Content;
        if(spn==null) continue;
        var(ox,oy,sw,sh)=Spr(spn);
        var(rsx,rsy)=RScale(n);
        int type=SlopeType(n);
        float x0=o.X-ox*rsx, y0=o.Y-oy*rsy, x1=x0+sw*rsx, y1=y0+sh*rsy;
        int rx=(int)Math.Round(Math.Min(x0,x1)), ry=(int)Math.Round(Math.Min(y0,y1));
        int rw=(int)Math.Round(Math.Abs(x1-x0)), rh=(int)Math.Round(Math.Abs(y1-y0));
        if(type==0){ // clamp rectangles (including the 8000px walls) to the room
            if(rx<0){ rw+=rx; rx=0; }
            if(ry<0){ rh+=ry; ry=0; }
            if(rx+rw>RW) rw=RW-rx;
            if(ry+rh>RH) rh=RH-ry;
        }
        if(rw>0 && rh>0) rects.Add((rx,ry,rw,rh,type));
    }
    string macro = rn.Replace("room_","").ToUpper();
    var sb=new StringBuilder();
    sb.AppendLine($"// {rn} collision — Undertale's real solids (room {RW}x{RH}). {{x,y,w,h,type}};");
    sb.AppendLine($"// type 0=rect, 1=sur,2=sul,3=sdr,4=sdl (20x20 slope triangles). Auto-generated.");
    sb.AppendLine($"static const short {macro}_SOLIDS[][5] = {{");
    foreach(var r in rects) sb.AppendLine($"    {{ {r.Item1}, {r.Item2}, {r.Item3}, {r.Item4}, {r.Item5} }},");
    sb.AppendLine("};");
    sb.AppendLine($"#define {macro}_SOLID_COUNT {rects.Count}");
    File.WriteAllText(Path.Combine(outDir,$"solids_{rn.Replace("room_","")}.h"), sb.ToString());
    Console.WriteLine($"{rn}: {rects.Count} rects ({RW}x{RH})");
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\room_dims.txt", dims.ToString());

// --- also dump door transition logic ---
var gctx = new GlobalDecompileContext(Data);
var dset = Data.ToolInfo.DecompilerSettings;
string DD(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return "(none)"; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,dset).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
var dsb=new StringBuilder();
foreach(var ob in new[]{"obj_doorparent","obj_doorA","obj_doorB","obj_doorC","obj_doorD"}){
  dsb.AppendLine("==== "+ob+" ====");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null && x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){
    dsb.AppendLine("-- "+c.Name.Content+" --"); dsb.AppendLine(DD(c.Name.Content));
  }
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\door_logic.txt", dsb.ToString());
Console.WriteLine("door logic written");
