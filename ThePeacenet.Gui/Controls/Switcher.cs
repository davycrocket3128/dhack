using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using MonoGame.Extended;

namespace ThePeacenet.Gui.Controls
{
    public class Switcher : ItemsControl
    {
        private int _activeIndex = 0;

        public override IEnumerable<Control> Children
        {
            get
            {
                if (Items.Count > 0)
                    yield return Items[ActiveIndex];
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
