using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui
{
    public class GuiSkin
    {
        [ContentSerializer(CollectionItemName = "Element")]
        public List<SkinElement> Elements { get; set; }
    }

    public class SkinElement
    {
        public string Control { get; set; }

        [ContentSerializer(CollectionItemName = "Style")]
        public List<SkinStyle> Styles { get; set; }
    }

    public class SkinStyle
    {
        [ContentSerializer(Optional = true)]
        public string Name { get; set; } = "";

        [ContentSerializer(CollectionItemName = "State")]
        public List<SkinState> States { get; set; } = new List<SkinState>();
    }

    public class SkinState
    {
        [ContentSerializer(Optional = true)]
        public string Name { get; set; } = "Default";

        [ContentSerializer(CollectionItemName = "Brush", Optional = true)]
        public List<BrushData> Brushes { get; set; } = new List<BrushData>();

        [ContentSerializer(CollectionItemName = "Property", Optional = true)]
        public List<ControlProperty> Properties { get; set; } = new List<ControlProperty>();

        [ContentSerializer(CollectionItemName = "Font", Optional = true)]
        public List<Font> Fonts { get; set; } = new List<Font>();
    }

    public class Font
    {
        public string Property { get; set; }
        public string Path { get; set; }

        [ContentSerializer(Optional = true)]
        public float Size { get; set; } = 16;
    }

    public class ControlProperty
    {
        public string Name { get; set; }
        public object Value { get; set; }
    }

    public class BrushData
    {
        public string Name { get; set; }

        [ContentSerializer(Optional = true)]
        public BrushColorData Color { get; set; } = new BrushColorData { Red = 255, Green = 255, Blue = 255, Alpha = 255 };

        [ContentSerializer(Optional = true)]
        public string Texture { get; set; } = "";

        [ContentSerializer(Optional = true)]
        public BrushType BrushType { get; set; } = BrushType.Image;

        [ContentSerializer(Optional = true)]
        public Thickness Margin { get; set; } = new Thickness(0);

        [ContentSerializer(Optional = true)]
        public Size2 ImageSize { get; set; } = Size2.Empty;

        public Brush CreateBrush(ContentManager content)
        {
            Texture2D texture = null;
            if (!string.IsNullOrWhiteSpace(Texture))
                texture = content.Load<Texture2D>(Texture);

            return new Brush(
                    new Microsoft.Xna.Framework.Color(Color.Red, Color.Green, Color.Blue, Color.Alpha),
                    texture,
                    Margin,
                    ImageSize,
                    BrushType
                );
        }
    }

    public class BrushColorData
    {
        public int Red { get; set; }
        public int Green { get; set; }
        public int Blue { get; set; }
        public int Alpha { get; set; }
    }
}
