using System;
using System.Linq;
foreach (var n in new[] { "obj_solidsmall", "obj_solidlong", "obj_solidtall", "obj_sur", "obj_mainchara" }) {
    var o = Data.GameObjects.FirstOrDefault(g => g.Name?.Content == n);
    if (o == null) { Console.WriteLine(n + ": MISSING"); continue; }
    var s = o.Sprite; var m = o.CollisionShape;
    Console.WriteLine($"{n}: sprite={s?.Name?.Content} {s?.Width}x{s?.Height} margins[L{s?.MarginLeft} R{s?.MarginRight} T{s?.MarginTop} B{s?.MarginBottom}] origin={s?.OriginX},{s?.OriginY} solid={o.Solid} shape={m}");
}
