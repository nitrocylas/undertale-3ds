using System;
using System.IO;
using System.Linq;
using System.Text;

string outDir = @"C:\Users\jojoc\undertale-3ds\extracted\slice3";
Directory.CreateDirectory(outDir);
var sb = new StringBuilder();

// Room order (index -> name) for the first chunk, to see what follows room_intromenu.
sb.AppendLine("== ROOM ORDER (first 30) ==");
for (int i = 0; i < Math.Min(30, Data.Rooms.Count); i++)
    sb.AppendLine($"  [{i}] {Data.Rooms[i].Name?.Content}");

// Which rooms contain flowey / the flower bed?
sb.AppendLine("\n== rooms containing obj_flowey / flowerbed / lie sprite objects ==");
foreach (var room in Data.Rooms)
{
    var objs = room.GameObjects.Select(o => o.ObjectDefinition?.Name?.Content ?? "").ToList();
    if (objs.Any(n => n.Contains("flowey") || n.Contains("flowerbed") || n.Contains("_lie") || n.Contains("bed")))
        sb.AppendLine($"  {room.Name?.Content}: " + string.Join(", ", objs.Where(n => n.Contains("flowey") || n.Contains("bed") || n.Contains("lie"))));
}
File.WriteAllText(Path.Combine(outDir, "firstroom.txt"), sb.ToString());
Console.WriteLine(sb.ToString());
