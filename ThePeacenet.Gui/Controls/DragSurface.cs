using Microsoft.Xna.Framework;
using MonoGame.Extended.Input.InputListeners;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class DragSurface : ContentControl
    {
        private bool _dragging = false;
        private Point _lastPoint = Point.Zero;

        public override bool OnPointerDown(IGuiContext context, PointerEventArgs args)
        {
            if(args.Button == MouseButton.Left)
            {
                _dragging = true;
                _lastPoint = args.Position;                
            }
            return base.OnPointerDown(context, args);
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            if (_dragging)
            {
                var delta = context.CursorPosition - _lastPoint;
                FireDragEvent(delta);
                _lastPoint = context.CursorPosition;
            }
            base.Update(context, deltaSeconds);
        }

        private void FireDragEvent(Point delta)
        {
            OnPointerDrag(delta);
            PointerDrag?.Invoke(this, new DragSurfaceEventArgs(delta));
        }

        protected virtual void OnPointerDrag(Point delta) { }

        public override bool OnPointerUp(IGuiContext context, PointerEventArgs args)
        {
            if (args.Button == MouseButton.Left)
            {
                _dragging = false;
            }
            return base.OnPointerUp(context, args);
        }

        public event EventHandler<DragSurfaceEventArgs> PointerDrag;
    }

    public class DragSurfaceEventArgs : EventArgs
    {
        public Point Delta { get; private set; }

        public DragSurfaceEventArgs(Point delta)
        {
            Delta = delta;
        }
    }
}
