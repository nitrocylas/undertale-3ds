// Export room_ruins1 as tile data + its tilesets, so we can composite a flat background,
// plus its solid collision (scaled) and key markers/doors.
using System;
using System.IO;
using System.Linq;
using System.Text;
using UndertaleModLib.Util;
using UndertaleModLib.Models;

string dir = @"C:\Users\jojoc\undertale-3ds\extracted\slice4\ruins1";
Directory.CreateDirectory(dir);

var room = Data.Rooms.First(r => r.Name?.Content == "room_ruins1");
var sb = new StringBuilder();
sb.AppendLine($"SIZE {room.Width} {room.Height}");

// Tiles: dst_x dst_y src_x src_y w h tileset
var tilesets = new System.Collections.Generic.HashSet<string>();
foreach (var t in room.Tiles) {
    string ts = t.spriteMode ? t.SpriteDefinition?.Name?.Content : t.BackgroundDefinition?.Name?.Content;
    if (ts == null) continue;
    tilesets.Add(ts);
    sb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {ts}");
}
File.WriteAllText(Path.Combine(dir, "tiles.txt"), sb.ToString());
Console.WriteLine("tiles=" + room.Tiles.Count + " tilesets=" + string.Join(",", tilesets));

// Export each tileset's texture.
using (var w = new TextureWorker()) {
    foreach (var ts in tilesets) {
        var bg = Data.Backgrounds.FirstOrDefault(b => b.Name?.Content == ts);
        if (bg?.Texture != null) {
            w.ExportAsPNG(bg.Texture, Path.Combine(dir, ts + ".png"), null, false);
            Console.WriteLine($"tileset {ts}: {bg.Texture.SourceWidth}x{bg.Texture.SourceHeight}");
        }
    }
}

// Collision solids (scaled) + markers.
bool IsSolid(string n) => n != null && (n.Contains("solid") || n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
bool Mask(UndertaleGameObject o, out int l,out int t,out int ww,out int hh){
    l=t=ww=hh=0; var s=o?.Sprite; if(s==null)return false;
    l=(int)s.MarginLeft-(int)s.OriginX; t=(int)s.MarginTop-(int)s.OriginY;
    ww=(int)s.MarginRight-(int)s.MarginLeft+1; hh=(int)s.MarginBottom-(int)s.MarginTop+1; return ww>0&&hh>0;
}
var cb = new StringBuilder();
foreach (var o in room.GameObjects) {
    string n = o.ObjectDefinition?.Name?.Content;
    if (IsSolid(n) && Mask(o.ObjectDefinition, out int l,out int t,out int ww,out int hh)) {
        float sx=o.ScaleX, sy=o.ScaleY;
        cb.AppendLine($"SOLID {(int)(o.X+l*sx)} {(int)(o.Y+t*sy)} {(int)Math.Abs(ww*sx)} {(int)Math.Abs(hh*sy)}");
    }
    if (n != null && (n.Contains("mainchara") || n.Contains("doorA") || n.Contains("doorB") || n.Contains("marker")))
        cb.AppendLine($"OBJ {n} {o.X} {o.Y}");
}
File.WriteAllText(Path.Combine(dir, "collision.txt"), cb.ToString());
Console.WriteLine("wrote collision.txt");
