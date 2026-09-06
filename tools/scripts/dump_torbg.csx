using System; using System.IO; using System.Linq;
using UndertaleModLib.Util;
string tsDir=@"C:\Users\jojoc\undertale-3ds\extracted\slice4\tilesets"; Directory.CreateDirectory(tsDir);
using(var w=new TextureWorker()){
foreach(var rn in new[]{"room_torhouse1"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null)continue;
  Console.WriteLine($"== {rn} backgrounds ==");
  foreach(var b in room.Backgrounds){
    string bn=b.BackgroundDefinition?.Name?.Content;
    Console.WriteLine($"  bg={bn} enabled={b.Enabled} pos=({b.X},{b.Y}) tiledV={b.TiledVertically} tiledH={b.TiledHorizontally}");
    if(bn!=null && b.Enabled){ var bg=Data.Backgrounds.FirstOrDefault(x=>x.Name?.Content==bn); if(bg?.Texture!=null) w.ExportAsPNG(bg.Texture,Path.Combine(tsDir,bn+".png"),null,false); }
  }
}
}
Console.WriteLine("bgdone");
