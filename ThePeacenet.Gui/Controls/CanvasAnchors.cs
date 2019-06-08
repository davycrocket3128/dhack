using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public static class CanvasAnchors
    {
        public static readonly Vector2 TopLeft = Vector2.Zero;
        public static readonly Vector2 TopCenter = new Vector2(0.5f, 0);
        public static readonly Vector2 TopRight = new Vector2(1, 0);

        public static readonly Vector2 Left = new Vector2(0, 0.5f);
        public static readonly Vector2 Center = new Vector2(0.5f, 0.5f);
        public static readonly Vector2 Right = new Vector2(1, 0.5f);

        public static readonly Vector2 BottomLeft = new Vector2(0, 1f);
        public static readonly Vector2 BottomCenter = new Vector2(0.5f, 1);
        public static readonly Vector2 BottomRight = new Vector2(1, 1f);

    }
}
