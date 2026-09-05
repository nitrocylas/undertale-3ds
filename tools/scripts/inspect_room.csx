// Inspect the first Ruins rooms so we know what to render (bg, tiles, objects, exits).
using System;
using System.IO;
using System.Linq;
using System.Text;

string outDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3";
Directory.CreateDirectory(outDir);

string[] rooms = { "room_area1", "room_area1_2" };
var sb = new StringBuilder();
foreach (var rn in rooms)
{
    var room = Data.Rooms.FirstOrDefault(r => r.Name?.Content == rn);
    if (room == null) { sb.AppendLine("MISSING " + rn); continue; }
    sb.AppendLine("==== " + rn + " ====");
    sb.AppendLine($"size {room.Width}x{room.Height}  bgColor=0x{room.BackgroundColor:X8}  persistent={room.Persistent}");

    sb.AppendLine("-- backgrounds --");
    foreach (var bg in room.Backgrounds)
        if (bg.Enabled || bg.BackgroundDefinition != null)
            sb.AppendLine($"  bg def={bg.BackgroundDefinition?.Name?.Content ?? "none"} pos={bg.X},{bg.Y} tiled={bg.TiledHorizontally}/{bg.TiledVertically} enabled={bg.Enabled}");

    sb.AppendLine($"-- tiles ({room.Tiles.Count}) --");
    foreach (var t in room.Tiles.Take(40))
        sb.AppendLine($"  tile dst={t.X},{t.Y} src={t.SourceX},{t.SourceY} {t.Width}x{t.Height} from={(t.spriteMode ? t.SpriteDefinition?.Name?.Content : t.BackgroundDefinition?.Name?.Content)}");
    if (room.Tiles.Count > 40) sb.AppendLine("  ...(more)");

    sb.AppendLine($"-- objects ({room.GameObjects.Count}) --");
    foreach (var o in room.GameObjects)
        sb.AppendLine($"  {o.ObjectDefinition?.Name?.Content} @ {o.X},{o.Y}");

    var cc = room.CreationCodeId;
    if (cc != null) sb.AppendLine("-- has room creation code --");
    sb.AppendLine();
}
File.WriteAllText(Path.Combine(outDir, "rooms_info.txt"), sb.ToString());
Console.WriteLine("wrote rooms_info.txt");
Console.WriteLine(sb.ToString().Substring(0, Math.Min(400, sb.Length)));
