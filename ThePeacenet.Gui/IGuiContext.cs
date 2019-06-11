using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using SpriteFontPlus;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Gui
{
    public interface IGuiContext
    {
        Control FocusedControl { get; }
        Point CursorPosition { get; }
        DynamicSpriteFont DefaultFont { get; }
        void GetBackBufferData<T>(Rectangle rect, T[] data) where T: struct;

        void SetFocus(Control control);

        Texture2D CreateTexture(int width, int height);
    }
}
