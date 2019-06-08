using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui;

namespace ThePeacenet.Desktop
{
    public abstract class WindowTheme
    {
        public static WindowTheme Current { get; set; }

        public abstract Brush BackgroundBrush { get; }
        public abstract Thickness ClientBorderMargin { get; }
        public abstract Color TitleTextColor { get; }
        public abstract Color WindowIconColor { get; }
        public abstract int WindowIconSize { get; }
    }
}
