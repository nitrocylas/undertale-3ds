// Extract Undertale's REAL solid collision for the Ruins rooms, applying the runtime scale
// overrides from each solid's Create event (solidlong -> xscale 400 = wide wall,
// solidtall -> yscale 400 = tall wall). World rects are clamped to the room.
using System;
using System.IO;
using System.Linq;
using System.Text;
using UndertaleModLib.Models;

string[] rooms = { "room_ruins5","room_ruins6","room_ruins7","room_ruins8","room_ruins9","room_ruins10","room_ruins11","room_ruins12" };
string outDir = @"C:\Users\jojoc\undertale-3ds\include";

// Sprite origin+size for the solid sprites (mask == full sprite here).
(int ox, int oy, int w, int h) SpriteInfo(string sname) {
    var s = Data.Sprites.FirstOrDefault(x => x.Name?.Content == sname);
    if (s == null) return (0, 0, 20, 20);
    return ((int)s.OriginX, (int)s.OriginY, (int)s.Width, (int)s.Height);
}

// The runtime scale each solid type forces in its Create event.
(float sx, float sy) RuntimeScale(string obj) {
    switch (obj) {
        case "obj_solidlong":     return (400f, 1f);
        case "obj_solidlongleft": return (-400f, 1f); // extends left
        case "obj_solidtall":     return (1f, 400f);
        default:                  return (1f, 1f);     // solidsmall, sur/sul/sdr/sdl
    }
}
bool IsSolid(string n) => n != null && (n.Contains("solid") || n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
// Slope type: 0 = full rect, 1 = sur, 2 = sul, 3 = sdr, 4 = sdl.
int SlopeType(string n) {
    switch (n) { case "obj_sur": return 1; case "obj_sul": return 2;
                 case "obj_sdr": return 3; case "obj_sdl": return 4; default: return 0; }
}

foreach (var rn in rooms) {
    var room = Data.Rooms.FirstOrDefault(r => r.Name?.Content == rn);
    if (room == null) continue;
    int RW = (int)room.Width, RH = (int)room.Height;
    var rects = new System.Collections.Generic.List<(int,int,int,int,int)>();
    foreach (var o in room.GameObjects) {
        string n = o.ObjectDefinition?.Name?.Content;
        if (!IsSolid(n)) continue;
        var spr = o.ObjectDefinition.Sprite; if (spr == null) continue;
        var (ox, oy, sw, sh) = SpriteInfo(spr.Name?.Content);
        var (rsx, rsy) = RuntimeScale(n);
        float sx = rsx, sy = rsy; // Create overrides the per-instance scale
        int type = SlopeType(n);
        float x0 = o.X - ox * sx;
        float y0 = o.Y - oy * sy;
        float x1 = x0 + sw * sx;
        float y1 = y0 + sh * sy;
        int rx = (int)Math.Round(Math.Min(x0, x1)), ry = (int)Math.Round(Math.Min(y0, y1));
        int rw = (int)Math.Round(Math.Abs(x1 - x0)), rh = (int)Math.Round(Math.Abs(y1 - y0));
        // Clamp full rects to the room; slopes stay 20x20 (their triangle math needs it).
        if (type == 0) {
            if (rx < 0) { rw += rx; rx = 0; }
            if (ry < 0) { rh += ry; ry = 0; }
            if (rx + rw > RW) rw = RW - rx;
            if (ry + rh > RH) rh = RH - ry;
        }
        if (rw > 0 && rh > 0) rects.Add((rx, ry, rw, rh, type));
    }
    string macro = rn.Replace("room_", "").ToUpper();
    var sb = new StringBuilder();
    sb.AppendLine($"// {rn} solid collision. Each: {{x,y,w,h,type}}. type 0=rect, 1=sur,2=sul,3=sdr,4=sdl (20x20 triangles).");
    sb.AppendLine($"static const short {macro}_SOLIDS[][5] = {{");
    foreach (var r in rects) sb.AppendLine($"    {{ {r.Item1}, {r.Item2}, {r.Item3}, {r.Item4}, {r.Item5} }},");
    sb.AppendLine("};");
    sb.AppendLine($"#define {macro}_SOLID_COUNT {rects.Count}");
    File.WriteAllText(Path.Combine(outDir, $"solids_{rn.Replace("room_", "")}.h"), sb.ToString());
    Console.WriteLine($"{rn}: {rects.Count} solid rects (room {RW}x{RH})");
}
