using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using MonoGame.Extended;

namespace ThePeacenet.Gui.Controls
{
    public class ProgressBar : Control
    {
        private float _marqueeProgress = 0.0f;
        private float _marqueeWidth = 0.2f;

        public override IEnumerable<Control> Children => Enumerable.Empty<Control>();

        public Brush ProgressBrush { get; set; } = new Brush(Color.White);
        public float Progress { get; set; } = 0.0f;
        public bool Marquee { get; set; } = false;

        public ProgressBar()
        {
            MinHeight = 4;
        }

        public override Size2 GetContentSize(IGuiContext context)
        {
            return Size2.Empty;
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            base.Update(context, deltaSeconds);
            if(Marquee)
            {
                _marqueeProgress += deltaSeconds;
                if (_marqueeProgress >= 1)
                    _marqueeProgress = -_marqueeWidth;
            }
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            base.Draw(context, renderer, deltaSeconds);

            if (Marquee)
            {
                float marqueeX = BoundingRectangle.X + (BoundingRectangle.Width * _marqueeProgress);
                float marqueeWidth = BoundingRectangle.Width * _marqueeWidth;
                renderer.DrawBrush(new Rectangle((int)marqueeX, BoundingRectangle.Y, (int)marqueeWidth, BoundingRectangle.Height), ProgressBrush);
            }
            else
            {
                float progressWidth = BoundingRectangle.Width * Progress;
                renderer.DrawBrush(new Rectangle(BoundingRectangle.X, BoundingRectangle.Y, (int)progressWidth, BoundingRectangle.Height), ProgressBrush);
            }
        }
    }
}
