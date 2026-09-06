using System; using System.IO; using System.Linq; using UndertaleModLib.Util;
string tsDir=@"C:\Users\jojoc\undertale-3ds\extracted\slice4\tilesets"; Directory.CreateDirectory(tsDir);
using(var w=new TextureWorker()){
foreach(var bn in new[]{"bg_parlor1","bg_parlorstairs","bg_livingroom","bg_torhallway","bg_torhallmirror","bg_ruinsplaceholder2"}){
  var bg=Data.Backgrounds.FirstOrDefault(x=>x.Name?.Content==bn);
  if(bg?.Texture!=null){ w.ExportAsPNG(bg.Texture,Path.Combine(tsDir,bn+".png"),null,false); Console.WriteLine($"{bn}: {bg.Texture.SourceWidth}x{bg.Texture.SourceHeight}"); }
  else Console.WriteLine(bn+": NULL");
}
}
