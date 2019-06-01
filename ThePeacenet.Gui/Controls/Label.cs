using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class Label : ContentControl
    {
        public Label()
        {
        }

        public Label(string text = null)
        {
            Content = text ?? string.Empty;
        }
    }
}
