using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class CheckLabel : CompositeControl
    {
        private StackPanel _stacker = new StackPanel();
        private CheckBox _checkBox = new CheckBox();
        private Label _label = new Label();

        public override Control Template => _stacker;

        public CheckLabel()
        {
            _stacker.Orientation = Orientation.Horizontal;
            _stacker.Items.Add(_checkBox);
            _stacker.Items.Add(_label);
        }

        public bool Checked
        {
            get => _checkBox.Checked;
            set => _checkBox.Checked = value;
        }

        public object Content
        {
            get => _label.Content;
            set => _label.Content = value;
        }
    }
}
