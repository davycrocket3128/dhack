using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class ControlCollection : ElementCollection<Control, Control>
    {
        public ControlCollection()
            : base(null)
        {
        }

        public ControlCollection(Control parent)
            : base(parent)
        {
        }
    }
}
