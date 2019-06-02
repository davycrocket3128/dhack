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

        private readonly string OriginalLocationProperty = "CanvasOriginalLocation";

        protected override void Layout(IGuiContext context, Rectangle rectangle)
        {
            foreach (var control in Items)
            {
                if (control.GetAttachedProperty(OriginalLocationProperty) == null)
                {
                    var anchor = control.GetAttachedProperty(AnchorProperty) as Vector2? ?? Vector2.Zero;
                    var alignment = control.GetAttachedProperty(AlignmentProperty) as Vector2? ?? Vector2.Zero;

                    var actualSize = control.CalculateActualSize(context);


                    var position = control.Position;

                    control.SetAttachedProperty(OriginalLocationProperty, position);

                    // Calculate the control's position based on its alignment value.
                    position.X -= (int)(actualSize.Width * alignment.X);
                    position.Y -= (int)(actualSize.Height * alignment.Y);

                    // get the anchor position.
                    var anchorPosX = rectangle.X + (rectangle.Width * anchor.X);
                    var anchorPosY = rectangle.Y + (rectangle.Height * anchor.Y);

                    // Modify the control position based on it.
                    position.X = (int)(anchorPosX + position.X);
                    position.Y = (int)(anchorPosY + position.Y);

                    PlaceControl(context, control, position.X, position.Y, actualSize.Width, actualSize.Height);
                }
            }
        }

        public override Size2 GetContentSize(IGuiContext context)
        {
            return new Size2();
        }

        public static readonly string AnchorProperty = "Anchor";
        public static readonly string AlignmentProperty = "Alignment";

        public override Type GetAttachedPropertyType(string propertyName)
        {
            if (string.Equals(AnchorProperty, propertyName, StringComparison.OrdinalIgnoreCase) || string.Equals(AlignmentProperty, propertyName, StringComparison.OrdinalIgnoreCase))
                return typeof(Vector2);

            return base.GetAttachedPropertyType(propertyName);
        }
    }
}
