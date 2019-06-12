using System;
using Microsoft.Xna.Framework;
using MonoGame.Extended;

namespace ThePeacenet.Gui.Controls
{
    public class Overlay : LayoutControl
    {
        public override Size2 GetContentSize(IGuiContext context)
        {
            float w = 0;
            float h = 0;

            foreach(var child in Children)
            {
                var size = child.CalculateActualSize(context);
                w = Math.Max(w, size.Width);
                h = Math.Max(h, size.Height);
            }

            return new Size2(w, h);
        }

        protected override void Layout(IGuiContext context, Rectangle rectangle)
        {
            foreach(var child in Children)
            {
                var hAlign = child.HorizontalAlignment;
                var vAlign = child.VerticalAlignment;

                var actualSize = child.CalculateActualSize(context);

                float x = rectangle.Left;
                float y = rectangle.Top;
                float w = actualSize.Width;
                float h = actualSize.Height;

                switch(hAlign)
                {
                    case HorizontalAlignment.Centre:
                        x += (rectangle.Width - w) / 2;
                        break;
                    case HorizontalAlignment.Right:
                        x += (rectangle.Width - w);
                        break;
                    case HorizontalAlignment.Stretch:
                        w = rectangle.Width;
                        break;
                }

                switch (vAlign)
                {
                    case VerticalAlignment.Centre:
                        y += (rectangle.Height - h) / 2;
                        break;
                    case VerticalAlignment.Bottom:
                        y += (rectangle.Height - h);
                        break;
                    case VerticalAlignment.Stretch:
                        h = rectangle.Height;
                        break;
                }

                PlaceControl(context, child, x, y, w, h);
            }
        }
    }
}
