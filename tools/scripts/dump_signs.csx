using System; using System.IO; using System.Linq; using System.Text;
var gctx = new GlobalDecompileContext(Data);
var ds = Data.ToolInfo.DecompilerSettings;
string D(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,ds).DecompileToString();}catch{return null;}}
var sb=new StringBuilder();
// 1) All readable/sign object instances per ruins room, with position.
string[] rooms={"room_ruins1","room_ruins2","room_ruins3","room_ruins4","room_ruins5","room_ruins6","room_ruins7","room_ruins7A","room_ruins8","room_ruins9","room_ruins10","room_ruins11","room_ruins12"};
var readObjs=new System.Collections.Generic.HashSet<string>();
foreach(var rn in rooms){
  var room=Data.Rooms.FirstOrDefault(r=>r.Name?.Content==rn); if(room==null)continue;
  foreach(var o in room.GameObjects){
    string n=o.ObjectDefinition?.Name?.Content;
    if(n!=null&&(n.Contains("readable")||n.Contains("sign")||n.Contains("interactable")||n.Contains("plaque"))){
      sb.AppendLine($"{rn}: {n} @({o.X},{o.Y})");
      readObjs.Add(n);
    }
  }
}
sb.AppendLine();
// 2) Decompile each readable object's events to see which text keys it shows.
foreach(var ob in readObjs.OrderBy(x=>x)){
  sb.AppendLine("==== "+ob+" ====");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){
    var t=D(c.Name.Content); if(t==null)continue;
    sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t);
  }
  sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\signs.txt",sb.ToString());
Console.WriteLine("done");
