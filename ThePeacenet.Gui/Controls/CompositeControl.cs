using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using MonoGame.Extended;

namespace ThePeacenet.Gui.Controls
{
    public abstract class CompositeControl : Control
    {
        protected bool IsDirty { get; set; } = true;

        public abstract Control Template { get; }

        public override IEnumerable<Control> Children
        {
            get
            {
                if (Template != null)
                    yield return Template;
            }
        }

        public override void InvalidateMeasure()
        {
            IsDirty = true;
            base.InvalidateMeasure();
        }

        public override Size2 GetContentSize(IGuiContext context)
        {
            if (Template == null) return Size2.Empty;

            return Template.CalculateActualSize(context);
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            var control = Template;

            if (control != null)
            {
                if (IsDirty)
                {
                    control.Parent = this;
                    control.ActualSize = ContentRectangle.Size;
                    control.Position = new Point(Padding.Left, Padding.Top);
                    control.InvalidateMeasure();
                    IsDirty = false;
                }
            }
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            base.Draw(context, renderer, deltaSeconds);

            var control = Template;
            control?.Draw(context, renderer, deltaSeconds);
        }
    }
}
