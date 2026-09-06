using System; using System.IO; using System.Linq; using System.Text;
var sb=new StringBuilder();
bool Skip(string n)=> n==null || n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl"
    ||n.Contains("door")||n.Contains("fakewater")||n=="obj_overworldcontroller"||n=="obj_mainchara";
foreach(var rn in new[]{"room_ruins8","room_ruins9","room_ruins10","room_ruins11","room_ruins12"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null){sb.AppendLine(rn+" MISSING");continue;}
  sb.AppendLine($"==== {rn} {room.Width}x{room.Height} ====");
  var counts=new System.Collections.Generic.Dictionary<string,int>();
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content; if(Skip(n)) continue;
    sb.AppendLine($"  {n} @({o.X},{o.Y})  spr={o.ObjectDefinition?.Sprite?.Name?.Content}");
  }
  sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\puzzles.txt",sb.ToString());
Console.WriteLine("done");
