using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MonoGame.Extended;

namespace ThePeacenet.Gui.Controls
{
    public class Image : Control
    {
        public override IEnumerable<Control> Children => Enumerable.Empty<Control>();

        public override Size2 GetContentSize(IGuiContext context)
        {
            return BackgroundBrush.ImageSize;
        }
    }
}
