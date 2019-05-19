using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui
{
    public enum BrushType
    {
        None,
        Image,
        Box,
        Border
    }

    public struct Brush
    {
        public Color BrushColor;
        public Texture2D Texture;
        public Thickness Margin;
        public Size2 ImageSize;
        public BrushType BrushType;

        public Brush(Color brushColor, Texture2D texture, Thickness margin, Size2 size, BrushType type)
        {
            BrushColor = brushColor;
            Texture = texture;
            Margin = margin;
            ImageSize = size;
            BrushType = type;
        }

        public Brush(Color color) : this(color, null, new Thickness(0), Size2.Empty, BrushType.Image) { }
        public Brush(Texture2D texture)
        {
            BrushColor = Color.White;
            Texture = texture;

            BrushType = BrushType.Image;

            Margin = new Thickness(0);

            if(Texture != null)
            {
                ImageSize = new Size2(Texture.Width, Texture.Height);
            }
            else
            {
                ImageSize = Size2.Empty;
            }
        }

        public Brush(Texture2D texture, int uniformSize) : this(texture)
        {
            ImageSize = new Size2(uniformSize, uniformSize);
        }

        public static Brush None => new Brush(Color.Transparent, null, new Thickness(0), Size2.Empty, BrushType.None);
    }
}
