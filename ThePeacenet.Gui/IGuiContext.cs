using Microsoft.Xna.Framework;
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
        void GetBackBufferData(Rectangle rect, byte[] data);

        void SetFocus(Control control);
    }
}
