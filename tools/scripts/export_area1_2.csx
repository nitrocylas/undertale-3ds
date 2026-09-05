// Export Flowey's room (room_area1_2): background + solid collision + key markers.
using System;
using System.IO;
using System.Linq;
using System.Text;
using UndertaleModLib.Util;
using UndertaleModLib.Models;

string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3\area1_2";
Directory.CreateDirectory(dir);

using (var worker = new TextureWorker()) {
    var bg = Data.Backgrounds.FirstOrDefault(b => b.Name?.Content == "bg_floweyglow");
    if (bg?.Texture != null) {
        worker.ExportAsPNG(bg.Texture, Path.Combine(dir, "bg_floweyglow.png"), null, false);
        Console.WriteLine($"bg_floweyglow {bg.Texture.SourceWidth}x{bg.Texture.SourceHeight}");
    } else Console.WriteLine("MISSING bg_floweyglow");
}

bool IsSolid(string n) => n != null && (n.Contains("solid") || n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
bool MaskBox(UndertaleGameObject o, out int l,out int t,out int w,out int h){
    l=t=w=h=0; var s=o?.Sprite; if(s==null)return false;
    l=(int)s.MarginLeft-(int)s.OriginX; t=(int)s.MarginTop-(int)s.OriginY;
    w=(int)s.MarginRight-(int)s.MarginLeft+1; h=(int)s.MarginBottom-(int)s.MarginTop+1; return w>0&&h>0;
}
var room = Data.Rooms.First(r => r.Name?.Content == "room_area1_2");
var sb = new StringBuilder();
sb.AppendLine($"// room_area1_2 size {room.Width}x{room.Height}");
sb.AppendLine("static const short AREA1_2_SOLIDS[][4] = {");
int c=0;
foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(!IsSolid(n)) continue;
    if(!MaskBox(o.ObjectDefinition, out int l,out int t,out int w,out int h)) continue;
    float sx=o.ScaleX, sy=o.ScaleY;
    int wx=(int)Math.Round(o.X+l*sx), wy=(int)Math.Round(o.Y+t*sy);
    int ww=(int)Math.Round(Math.Abs(w*sx)), wh=(int)Math.Round(Math.Abs(h*sy));
    sb.AppendLine($"    {{ {wx}, {wy}, {ww}, {wh} }},");
    c++;
}
sb.AppendLine("};");
sb.AppendLine($"#define AREA1_2_SOLID_COUNT {c}");
// key markers
foreach(var name in new[]{"obj_mainchara","obj_floweytalker1","obj_floweytrigger","obj_doorB","obj_doorAmusicfade","obj_markerA","obj_markerB"}){
    var inst = room.GameObjects.Where(o=>o.ObjectDefinition?.Name?.Content==name).Select(o=>$"({o.X},{o.Y})");
    sb.AppendLine($"// {name}: {string.Join(" ", inst)}");
}
File.WriteAllText(Path.Combine(dir,"area1_2_info.txt"), sb.ToString());
Console.WriteLine(sb.ToString());
