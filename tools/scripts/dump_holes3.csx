using System; using System.IO; using System.Linq; using System.Text;
using UndertaleModLib.Util;
string outDir=@"C:\Users\jojoc\undertale-3ds\extracted\holes"; Directory.CreateDirectory(outDir);
using(var w=new TextureWorker()){
  foreach(var sn in new[]{"spr_holemask","spr_hole","spr_event"}){
    var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==sn); if(s==null){Console.WriteLine("miss "+sn);continue;}
    for(int i=0;i<s.Textures.Count;i++) if(s.Textures[i]?.Texture!=null) w.ExportAsPNG(s.Textures[i].Texture, Path.Combine(outDir,$"{sn}_{i}.png"), null, true);
    Console.WriteLine($"{sn}: {s.Width}x{s.Height} frames {s.Textures.Count}");
  }
}
// ruins10 tiles (leaf path) + solids for context
var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content=="room_ruins10");
var sb=new StringBuilder();
sb.AppendLine($"SIZE {room.Width} {room.Height}");
var ts=new System.Collections.Generic.HashSet<string>();
foreach(var t in room.Tiles){ string s=t.spriteMode? t.SpriteDefinition?.Name?.Content : t.BackgroundDefinition?.Name?.Content; if(s==null)continue; ts.Add(s); sb.AppendLine($"TILE {t.X} {t.Y} {t.SourceX} {t.SourceY} {t.Width} {t.Height} {s}"); }
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\holes\ruins10_tiles.txt",sb.ToString());
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\holes\ruins10_tilesets.txt",string.Join("\n",ts));
Console.WriteLine("tiles done: "+string.Join(",",ts));
