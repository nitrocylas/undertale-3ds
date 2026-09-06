// Print the INTERNAL room order (what room_next/room_prev follow) and the code of the special
// door objects, so we can resolve where each Ruins door actually leads.
using System;
using System.IO;
using System.Linq;
using System.Text;

var sb = new StringBuilder();
sb.AppendLine("== INTERNAL ROOM ORDER (index : name) ==");
for (int i = 0; i < Data.Rooms.Count; i++) {
    string nm = Data.Rooms[i].Name?.Content ?? "?";
    if (nm.Contains("ruins") || nm.Contains("area1") || nm.Contains("torhouse") || nm.Contains("home"))
        sb.AppendLine($"  {i} : {nm}");
}
sb.AppendLine();

var gctx = new GlobalDecompileContext(Data);
var dsettings = Data.ToolInfo.DecompilerSettings;
string Decomp(string codeName){
    var c = Data.Code.FirstOrDefault(x=>x.Name?.Content==codeName);
    if(c==null) return "(no code "+codeName+")";
    try { return new Underanalyzer.Decompiler.DecompileContext(gctx, c, dsettings).DecompileToString(); }
    catch(Exception e){ return "(decompile failed: "+e.Message+")"; }
}
string[] doorObjs = { "obj_doorA","obj_doorB","obj_doorC","obj_doorD","obj_ruinsdoor1","obj_door_ruins13",
                      "obj_doorAmusicfade","obj_doorBmusicfade" };
foreach(var ob in doorObjs){
    sb.AppendLine($"==== {ob} ====");
    // collision-with-mainchara event usually carries the room_goto; also Create/Step.
    foreach(var suffix in new[]{"_Create_0","_Collision_", "_Alarm_0","_Step_0"}){
        var codes = Data.Code.Where(x=>x.Name?.Content!=null && x.Name.Content.StartsWith("gml_Object_"+ob+suffix)).ToList();
        foreach(var c in codes){
            sb.AppendLine($"  -- {c.Name.Content} --");
            sb.AppendLine(Decomp(c.Name.Content));
        }
    }
    sb.AppendLine();
}
File.WriteAllText(@"C:\Users\jojoc\undertale-3ds\extracted\room_order_doors.txt", sb.ToString());
Console.WriteLine("wrote room_order_doors.txt");
