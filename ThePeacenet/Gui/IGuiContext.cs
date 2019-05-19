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
        Vector2 CursorPosition { get; }
        DynamicSpriteFont DefaultFont { get; }

        void SetFocus(Control control);
    }
}
