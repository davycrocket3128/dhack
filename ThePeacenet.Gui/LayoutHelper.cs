using Microsoft.Xna.Framework;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Gui
{
    public enum HorizontalAlignment { Left, Right, Centre, Stretch }
    public enum VerticalAlignment { Top, Bottom, Centre, Stretch }

    public static class LayoutHelper
    {
        public static void PlaceControl(IGuiContext context, Control control, float x, float y, float width, float height)
        {
            var rectangle = new Rectangle((int)x, (int)y, (int)width, (int)height);
            var desiredSize = control.CalculateActualSize(context);
            var alignedRectangle = AlignRectangle(control.HorizontalAlignment, control.VerticalAlignment, desiredSize, rectangle);

            control.Position = new Point(control.Margin.Left + alignedRectangle.X, control.Margin.Top + alignedRectangle.Y);
            control.ActualSize = (Size2)alignedRectangle.Size - control.Margin.Size;
            control.InvalidateMeasure();
        }

        public static Rectangle AlignRectangle(HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment, Size2 size, Rectangle targetRectangle)
        {
            var x = GetHorizontalPosition(horizontalAlignment, size, targetRectangle);
            var y = GetVerticalPosition(verticalAlignment, size, targetRectangle);
            var width = horizontalAlignment == HorizontalAlignment.Stretch ? targetRectangle.Width : size.Width;
            var height = verticalAlignment == VerticalAlignment.Stretch ? targetRectangle.Height : size.Height;

            return new Rectangle(x, y, (int)width, (int)height);
        }

        public static int GetHorizontalPosition(HorizontalAlignment horizontalAlignment, Size2 size, Rectangle targetRectangle)
        {
            switch (horizontalAlignment)
            {
                case HorizontalAlignment.Stretch:
                case HorizontalAlignment.Left:
                    return targetRectangle.X;
                case HorizontalAlignment.Right:
                    return targetRectangle.Right - (int)size.Width;
                case HorizontalAlignment.Centre:
                    return targetRectangle.X + targetRectangle.Width / 2 - (int)size.Width / 2;
                default:
                    throw new ArgumentOutOfRangeException(nameof(horizontalAlignment), horizontalAlignment, $"{horizontalAlignment} is not supported");
            }
        }

        public static int GetVerticalPosition(VerticalAlignment verticalAlignment, Size2 size, Rectangle targetRectangle)
        {
            switch (verticalAlignment)
            {
                case VerticalAlignment.Stretch:
                case VerticalAlignment.Top:
                    return targetRectangle.Y;
                case VerticalAlignment.Bottom:
                    return targetRectangle.Bottom - (int)size.Height;
                case VerticalAlignment.Centre:
                    return targetRectangle.Y + targetRectangle.Height / 2 - (int)size.Height / 2;
                default:
                    throw new ArgumentOutOfRangeException(nameof(verticalAlignment), verticalAlignment, $"{verticalAlignment} is not supported");
            }
        }
    }
}
