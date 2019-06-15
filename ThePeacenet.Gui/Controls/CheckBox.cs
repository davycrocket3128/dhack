using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class CheckBox : Button
    {
        private ControlStyle _checkedStyle = null;
        private bool _checked = false;

        public CheckBox()
        {
            MinWidth = MinHeight = 16;
        }

        public bool Checked
        {
            get => _checked;
            set
            {
                if(_checked != value)
                {
                    _checked = value;
                    CheckedChanged?.Invoke(this, EventArgs.Empty);
                    OnPropertyChanged(nameof(Checked));
                    if(CheckedStyle != null)
                    {
                        CheckedStyle.ApplyIf(this, value);
                    }
                }
            }
        }

        public ControlStyle CheckedStyle
        {
            get => _checkedStyle;
            set
            {
                if(_checkedStyle != value)
                {
                    _checkedStyle = value;
                    if(_checkedStyle != null)
                    {
                        _checkedStyle.ApplyIf(this, Checked);
                    }
                }
            }
        }

        public event EventHandler CheckedChanged;

        public override bool OnPointerUp(IGuiContext context, PointerEventArgs args)
        {
            Checked = !Checked;
            return base.OnPointerUp(context, args);
        }
    }
}
