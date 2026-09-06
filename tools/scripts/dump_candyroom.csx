using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return "(none)"; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
(int ox,int oy,int w,int h) Spr(string s){ var sp=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==s); if(sp==null)return(0,0,20,20); return((int)sp.OriginX,(int)sp.OriginY,(int)sp.Width,(int)sp.Height);}
(float sx,float sy) RS(string o){switch(o){case "obj_solidlong":return(400f,1f);case "obj_solidlongleft":return(-400f,1f);case "obj_solidtall":return(1f,400f);default:return(1f,1f);}}
bool IsSolid(string n)=>n!=null&&(n.Contains("solid")||n=="obj_sur"||n=="obj_sul"||n=="obj_sdr"||n=="obj_sdl");
int ST(string n){switch(n){case "obj_sur":return 1;case "obj_sul":return 2;case "obj_sdr":return 3;case "obj_sdl":return 4;default:return 0;}}
var sb=new StringBuilder();
foreach(var rn in new[]{"room_ruins7A"}){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null){sb.AppendLine(rn+" NOT FOUND");continue;}
  int RW=(int)room.Width,RH=(int)room.Height; sb.AppendLine($"== {rn} {RW}x{RH} ==");
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(IsSolid(n)){ string spn=o.ObjectDefinition.Sprite?.Name?.Content; var(ox,oy,sw,sh)=Spr(spn); var(rsx,rsy)=RS(n); int ty=ST(n);
      float x0=o.X-ox*rsx,y0=o.Y-oy*rsy,x1=x0+sw*rsx,y1=y0+sh*rsy; int rx=(int)Math.Round(Math.Min(x0,x1)),ry=(int)Math.Round(Math.Min(y0,y1)),rw=(int)Math.Round(Math.Abs(x1-x0)),rh=(int)Math.Round(Math.Abs(y1-y0));
      if(ty==0){if(rx<0){rw+=rx;rx=0;}if(ry<0){rh+=ry;ry=0;}if(rx+rw>RW)rw=RW-rx;if(ry+rh>RH)rh=RH-ry;}
      sb.AppendLine($"  SOLID {n} box({rx},{ry},{rw},{rh},{ty})");
    } else sb.AppendLine($"  OBJ {n} @({o.X},{o.Y})");
  }
}
sb.AppendLine(); sb.AppendLine("==== obj_candydish1 code ====");
foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_obj_candydish1_"))){ sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(D(c.Name.Content)); }
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\candyroom.txt",sb.ToString());
Console.WriteLine("ok");
