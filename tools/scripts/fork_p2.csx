using System; using System.IO; using System.Linq; using System.Text;
using UndertaleModLib.Util;
var gctx = new GlobalDecompileContext(Data);
var dset = Data.ToolInfo.DecompilerSettings;
string Dec(string n){ var c=Data.Code.FirstOrDefault(x=>x.Name?.Content==n); if(c==null)return null; try{return new Underanalyzer.Decompiler.DecompileContext(gctx,c,dset).DecompileToString();}catch(Exception e){return "(fail "+e.Message+")";}}
string baseDir=@"C:\Users\jojoc\undertale-3ds";
var sb=new StringBuilder();
foreach(var ob in new[]{"obj_bigweb","obj_smallweb"}){
  sb.AppendLine("#### "+ob+" ####");
  foreach(var c in Data.Code.Where(x=>x.Name?.Content!=null&&x.Name.Content.StartsWith("gml_Object_"+ob+"_"))){ var t=Dec(c.Name.Content); if(t==null)continue; sb.AppendLine("-- "+c.Name.Content+" --"); sb.AppendLine(t); }
}
// dialoguer case 510 (napstablook pre-battle)
var st=Dec("gml_Script_SCR_TEXT");
if(st!=null){ int i=st.IndexOf("case 510:"); if(i>=0){ sb.AppendLine("#### SCR_TEXT case 510 ####"); sb.AppendLine(st.Substring(i, Math.Min(400,st.Length-i))); } }
File.WriteAllText(baseDir+@"\extracted\fork_dialogue.txt",sb.ToString());
// export overworld sprites padded, report dims
var er=new StringBuilder();
using(var w=new TextureWorker()){
  foreach(var sn in new[]{"spr_savepoint","spr_bigweb","spr_smallweb","spr_cheesetable","spr_mousehole","spr_smallfrog","spr_npc_sign","spr_interactable","spr_spidertable_items"}){
    var s=Data.Sprites.FirstOrDefault(x=>x.Name?.Content==sn); if(s==null){er.AppendLine(sn+" MISSING");continue;}
    er.AppendLine($"{sn} {s.Width}x{s.Height} origin({s.OriginX},{s.OriginY}) frames={s.Textures.Count}");
    if(s.Textures.Count>0 && s.Textures[0]?.Texture!=null) w.ExportAsPNG(s.Textures[0].Texture, baseDir+$@"\extracted\ow_{sn}.png", null, true);
  }
}
File.WriteAllText(baseDir+@"\extracted\fork_sprites.txt",er.ToString());
Console.WriteLine("p2 done");
