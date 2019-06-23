using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using MonoGame.Extended;

namespace ThePeacenet.Gui.Controls
{
    public class Switcher : LayoutControl
    {
        private int _activeIndex = 0;

        public Switcher()
        {
            Items.ItemRemoved = Items.ItemAdded = (ctrl) =>
            {
                OnPropertyChanged(nameof(Items));
            };
        }

        public override IEnumerable<Control> Children
        {
            get
            {
                if (Items.Count > 0)
                    yield return Items[ActiveIndex];
            }
        }

        protected override void Layout(IGuiContext context, Rectangle rectangle)
        {
            if(Items.Count > 0)
            {
                PlaceControl(context, Children.First(), rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height);
            }
        }

        public int ActiveIndex
        {
            get => MathHelper.Clamp(_activeIndex, 0, Items.Count - 1);
            set
            {
                if(_activeIndex != value)
                {
                    _activeIndex = value;
                    OnPropertyChanged(nameof(ActiveIndex));
                }
            }
        }

        public override Size2 GetContentSize(IGuiContext context)
        {
            if (Items.Count < 1)
                return Size2.Empty;

            return Items[ActiveIndex].CalculateActualSize(context);
        }
    }
}
