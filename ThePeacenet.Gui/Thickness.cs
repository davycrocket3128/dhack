using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui
{
    public struct Thickness
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;

        public int Width => Left + Right;
        public int Height => Top + Bottom;

        public Size2 Size => new Size2(Width, Height);

        public Thickness(int left, int top, int right, int bottom)
        {
            Left = left;
            Top = top;
            Right = right;
            Bottom = bottom;
        }

        public static implicit operator Thickness(int value)
        {
            return new Thickness(value);
        }

        public Thickness(int all) : this(all, all, all, all) { }

        public static bool operator ==(Thickness a, Thickness b)
        {
            return (a.Left == b.Left && a.Right == b.Right && a.Top == b.Top && a.Bottom == b.Bottom);
        }

        public static bool operator !=(Thickness a, Thickness b)
        {
            return !(a.Left == b.Left && a.Right == b.Right && a.Top == b.Top && a.Bottom == b.Bottom);
        }
    }
}