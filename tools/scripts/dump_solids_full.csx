// Diagnostic: for each Ruins room, print real dimensions and every solid instance with the
// runtime scale (Create overrides: solidlong xscale=400, solidtall yscale=400), the sprite
// size/origin, and the resulting world bbox. Also dumps door/warp + mainchara spawn.
using System;
using System.IO;
using System.Linq;
using System.Text;

string[] rooms = { "room_ruins1","room_ruins2","room_ruins3","room_ruins4","room_ruins5",
                   "room_ruins6","room_ruins7","room_ruins8","room_ruins9","room_ruins10",
                   "room_ruins11","room_ruins12" };
var sb = new StringBuilder();

(int ox,int oy,int w,int h) Spr(string s){
    var sp = Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s);
    if(sp==null) return (0,0,0,0);
    return ((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height);
}
(float sx,float sy) RScale(string o){
    switch(o){ case "obj_solidlong": return (400f,1f);
               case "obj_solidlongleft": return (-400f,1f);
               case "obj_solidtall": return (1f,400f);
               default: return (1f,1f); }
}
bool IsSolid(string n)=> n!=null && (n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
bool IsDoor(string n)=> n!=null && (n.Contains("door")||n.Contains("warp"));

foreach(var rn in rooms){
    var room = Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn);
    if(room==null){ sb.AppendLine($"== {rn} : NOT FOUND =="); continue; }
    sb.AppendLine($"== {rn} : {room.Width}x{room.Height} ==");
    foreach(var o in room.GameObjects){
        string n=o.ObjectDefinition?.Name?.Content;
        if(IsSolid(n)){
            string spn=o.ObjectDefinition.Sprite?.Name?.Content ?? "(nomask)";
            var(ox,oy,sw,sh)=Spr(spn);
            var(rsx,rsy)=RScale(n);
            float x0=o.X-ox*rsx, y0=o.Y-oy*rsy, x1=x0+sw*rsx, y1=y0+sh*rsy;
            int rx=(int)Math.Round(Math.Min(x0,x1)), ry=(int)Math.Round(Math.Min(y0,y1));
            int rw=(int)Math.Round(Math.Abs(x1-x0)), rh=(int)Math.Round(Math.Abs(y1-y0));
            sb.AppendLine($"  {n} @({o.X},{o.Y}) instScale({o.ScaleX:0.##},{o.ScaleY:0.##}) spr={spn}[{sw}x{sh} o{ox},{oy}] -> box({rx},{ry},{rw},{rh})");
        } else if(IsDoor(n) || n=="obj_mainchara"){
            sb.AppendLine($"  * {n} @({o.X},{o.Y})");
        }
    }
    sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\solids_full_dump.txt", sb.ToString());
Console.WriteLine("wrote solids_full_dump.txt");
