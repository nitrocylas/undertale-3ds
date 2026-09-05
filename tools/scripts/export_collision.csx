// Emit room_area1 solid collision as world-space rects (C array), plus Frisk's mask box.
// Collision rect for an instance = its object's sprite mask bbox, offset by instance pos.
using System;
using System.IO;
using System.Linq;
using System.Text;
using UndertaleModLib.Models;

string outDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3";
Directory.CreateDirectory(outDir);

// Object name -> is it a blocking solid?
bool IsSolid(string n) =>
    n != null && (n.Contains("solid") || n == "obj_sur" || n == "obj_sul" ||
                  n == "obj_sdr" || n == "obj_sdl");

// Get an object's collision sprite (Sprite, else nothing) and its mask bbox in object space.
bool MaskBox(UndertaleGameObject obj, out int left, out int top, out int w, out int h) {
    left = top = w = h = 0;
    var spr = obj?.Sprite;
    if (spr == null) return false;
    int ml = (int)spr.MarginLeft, mr = (int)spr.MarginRight, mt = (int)spr.MarginTop, mb = (int)spr.MarginBottom;
    left = ml - (int)spr.OriginX;
    top  = mt - (int)spr.OriginY;
    w = mr - ml + 1;
    h = mb - mt + 1;
    return w > 0 && h > 0;
}

var room = Data.Rooms.First(r => r.Name?.Content == "room_area1");
var sb = new StringBuilder();
sb.AppendLine("// Auto-generated collision rects for room_area1 (world space: x,y,w,h).");
sb.AppendLine("static const short ROOM_AREA1_SOLIDS[][4] = {");
int count = 0;
foreach (var o in room.GameObjects) {
    string name = o.ObjectDefinition?.Name?.Content;
    if (!IsSolid(name)) continue;
    if (!MaskBox(o.ObjectDefinition, out int l, out int t, out int w, out int h)) continue;
    // Undertale scales a 20x20 solid to make long/tall walls: apply instance scale.
    float sx = o.ScaleX, sy = o.ScaleY;
    float x0 = (int)o.X + l * sx, y0 = (int)o.Y + t * sy;
    float x1 = x0 + w * sx,       y1 = y0 + h * sy;
    int wx = (int)Math.Round(Math.Min(x0, x1)), wy = (int)Math.Round(Math.Min(y0, y1));
    int ww = (int)Math.Round(Math.Abs(x1 - x0)), wh = (int)Math.Round(Math.Abs(y1 - y0));
    sb.AppendLine($"    {{ {wx}, {wy}, {ww}, {wh} }}, // {name} scale {sx}x{sy}");
    count++;
}
sb.AppendLine("};");
sb.AppendLine($"#define ROOM_AREA1_SOLID_COUNT {count}");

// Frisk mask box (relative to draw origin/top-left).
var frisk = Data.GameObjects.First(g => g.Name?.Content == "obj_mainchara");
if (MaskBox(frisk, out int fl, out int ft, out int fw, out int fh))
    sb.AppendLine($"// Frisk mask: left={fl} top={ft} w={fw} h={fh}");

File.WriteAllText(Path.Combine(outDir, "collision_area1.h"), sb.ToString());
Console.WriteLine("solids=" + count);
Console.WriteLine(sb.ToString());
