﻿using Microsoft.Xna.Framework;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public enum Orientation
    {
        Horizontal,
        Vertical
    }

    public class StackPanel : LayoutControl
    {
        public StackPanel()
        {
        }

        public Orientation Orientation { get; set; } = Orientation.Vertical;
        public int Spacing { get; set; }

        public override Size2 GetContentSize(IGuiContext context)
        {
            var width = 0f;
            var height = 0f;

            foreach (var control in Items)
            {
                var actualSize = control.CalculateActualSize(context);

                switch (Orientation)
                {
                    case Orientation.Horizontal:
                        width += actualSize.Width;
                        height = actualSize.Height > height ? actualSize.Height : height;
                        break;
                    case Orientation.Vertical:
                        width = actualSize.Width > width ? actualSize.Width : width;
                        height += actualSize.Height;
                        break;
                    default:
                        throw new InvalidOperationException($"Unexpected orientation {Orientation}");
                }
            }

            width += Orientation == Orientation.Horizontal ? (Items.Count - 1) * Spacing : 0;
            height += Orientation == Orientation.Vertical ? (Items.Count - 1) * Spacing : 0;

            return new Size2(width, height);
        }

        protected override void Layout(IGuiContext context, Rectangle rectangle)
        {
            foreach (var control in Items)
            {
                var actualSize = control.CalculateActualSize(context);

                switch (Orientation)
                {
                    case Orientation.Vertical:
                        PlaceControl(context, control, rectangle.X, rectangle.Y, rectangle.Width, actualSize.Height);
                        rectangle.Y += (int)actualSize.Height + Spacing;
                        rectangle.Height -= (int)actualSize.Height;
                        break;
                    case Orientation.Horizontal:
                        PlaceControl(context, control, rectangle.X, rectangle.Y, actualSize.Width, rectangle.Height);
                        rectangle.X += (int)actualSize.Width + Spacing;
                        rectangle.Width -= (int)actualSize.Width;
                        break;
                    default:
                        throw new InvalidOperationException($"Unexpected orientation {Orientation}");
                }
            }
        }

        
    }
}
