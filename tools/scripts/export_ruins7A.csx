// Export several Ruins rooms (tiles + tilesets + solids + doors/markers) for compositing.
using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using UndertaleModLib.Util;
using UndertaleModLib.Models;

string[] rooms = { "room_ruins7A" };
string baseDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice4";
var allTilesets = new HashSet<string>();

bool IsSolid(string n) => n != null && (n.Contains("solid") || n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
bool Mask(UndertaleGameObject o, out int l,out int t,out int ww,out int hh){
    l=t=ww=hh=0; var s=o?.Sprite; if(s==null)return false;
    l=(int)s.MarginLeft-(int)s.OriginX; t=(int)s.MarginTop-(int)s.OriginY;
    ww=(int)s.MarginRight-(int)s.MarginLeft+1; hh=(int)s.MarginBottom-(int)s.MarginTop+1; return ww>0&&hh>0;
}

foreach (var rn in rooms) {
    var room = Data.Rooms.FirstOrDefault(r => r.Name?.Content == rn);
    if (room == null) { Console.WriteLine("MISSING " + rn); continue; }
    string dir = Path.Combine(baseDir, rn.Replace("room_", ""));
    Directory.CreateDirectory(dir);

    var sb = new StringBuilder();
    sb.AppendLine($"SIZE {room.Width} {room.Height}");
    foreach (var t in room.Tiles) {
        string ts = t.spriteMode ? t.SpriteDefinition?.Name?.Content : t.BackgroundDefinition?.Name?.Content;
        if (ts == null) continue;
        allTilesets.Add(ts);
        sb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {ts}");
    }
    File.WriteAllText(Path.Combine(dir, "tiles.txt"), sb.ToString());

    var cb = new StringBuilder();
    foreach (var o in room.GameObjects) {
        string n = o.ObjectDefinition?.Name?.Content;
        if (IsSolid(n) && Mask(o.ObjectDefinition, out int l,out int t,out int ww,out int hh)) {
            float sx=o.ScaleX, sy=o.ScaleY;
            cb.AppendLine($"SOLID {(int)(o.X+l*sx)} {(int)(o.Y+t*sy)} {(int)Math.Abs(ww*sx)} {(int)Math.Abs(hh*sy)}");
        }
        if (n != null && (n.Contains("mainchara")||n.Contains("door")||n.Contains("marker")||n.Contains("switch")||n.Contains("toriel")))
            cb.AppendLine($"OBJ {n} {o.X} {o.Y}");
    }
    File.WriteAllText(Path.Combine(dir, "collision.txt"), cb.ToString());
    Console.WriteLine($"{rn}: {room.Width}x{room.Height}, {room.Tiles.Count} tiles");
}

// Export every tileset used (into a shared folder).
string tsDir = Path.Combine(baseDir, "tilesets");
Directory.CreateDirectory(tsDir);
using (var w = new TextureWorker()) {
    foreach (var ts in allTilesets) {
        var bg = Data.Backgrounds.FirstOrDefault(b => b.Name?.Content == ts);
        if (bg?.Texture != null) w.ExportAsPNG(bg.Texture, Path.Combine(tsDir, ts + ".png"), null, false);
    }
}
Console.WriteLine("tilesets: " + string.Join(",", allTilesets));
