using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using ThePeacenet.Gui;

namespace ThePeacenet.Desktop
{
    public class SerenityWindowTheme : WindowTheme
    {
        private readonly Texture2D _windowBg = null;

        public SerenityWindowTheme(ContentManager content)
        {
            _windowBg = content.Load<Texture2D>("Gui/Textures/window");
        }

        public override Brush BackgroundBrush => new Brush(Color.White, _windowBg, new Thickness(5), MonoGame.Extended.Size2.Empty, BrushType.Box);

        public override Thickness ClientBorderMargin => new Thickness(2);

        public override Color TitleTextColor => Color.White;

        public override Color WindowIconColor => Color.White;

        public override int WindowIconSize => 16;
    }
}
