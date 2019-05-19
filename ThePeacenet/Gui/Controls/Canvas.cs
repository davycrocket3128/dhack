using Microsoft.Xna.Framework;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class Canvas : LayoutControl
    {
        public Canvas()
        {
        }

        protected override void Layout(IGuiContext context, Rectangle rectangle)
        {
            foreach (var control in Items)
            {
                var actualSize = control.CalculateActualSize(context);
                PlaceControl(context, control, control.Position.X, control.Position.Y, actualSize.Width, actualSize.Height);
            }
        }

        public override Size2 GetContentSize(IGuiContext context)
        {
            return new Size2();
        }
    }
}
