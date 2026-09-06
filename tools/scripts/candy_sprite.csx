using System; using System.Linq;
var o=Data.GameObjects.FirstOrDefault(x=>x.Name?.Content=="obj_candydish1");
Console.WriteLine("obj_candydish1 default sprite = "+(o?.Sprite?.Name?.Content ?? "(none)"));
foreach(var sn in new[]{"spr_candydish","spr_candydish2","spr_candydish_bad"}){
  var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==sn);
  if(s!=null) Console.WriteLine($"{sn}: {s.Width}x{s.Height} origin({s.OriginX},{s.OriginY}) frames={s.Textures.Count}");
}
