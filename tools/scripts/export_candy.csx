using System; using System.IO; using System.Linq;
using UndertaleModLib.Util;
string outDir=@"C:\Users\jojoc\undertale-3ds\extracted\candy"; Directory.CreateDirectory(outDir);
using(var w=new TextureWorker()){
  foreach(var sn in new[]{"spr_candydish","spr_candydish2"}){
    var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==sn); if(s==null){Console.WriteLine("miss "+sn);continue;}
    Console.WriteLine($"{sn}: {s.Width}x{s.Height} origin {s.OriginX},{s.OriginY} frames {s.Textures.Count}");
    for(int i=0;i<s.Textures.Count;i++) if(s.Textures[i]?.Texture!=null) w.ExportAsPNG(s.Textures[i].Texture, Path.Combine(outDir,$"{sn}_{i}.png"), null, true);
  }
}
