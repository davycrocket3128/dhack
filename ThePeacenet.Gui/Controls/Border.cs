using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class Border : CompositeControl
    {
        public Control Content { get; set; }

        public override Control Template => Content;
    }
}
