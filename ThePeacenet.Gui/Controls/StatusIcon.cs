using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class StatusIcon : CompositeControl
    {
        private Image _image = null;
        private Label _label = null;
        private StackPanel _holder = null;

        public StatusIcon()
        {
            _image = new Image();
            _label = new Label();
            _holder = new StackPanel
            {
                Orientation = Orientation.Horizontal
            };

            _holder.Items.Add(_image);
            _holder.Items.Add(_label);

            _image.VerticalAlignment = _label.VerticalAlignment = VerticalAlignment.Centre;
        }

        public Brush IconBrush { get => _image.BackgroundBrush; set => _image.BackgroundBrush = value; }
        public string Content { get => _label.Content.ToString(); set => _label.Content = value; }

        public override Control Template => _holder;
    }
}
