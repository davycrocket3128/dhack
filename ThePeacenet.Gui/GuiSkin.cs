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
        public string ControlType { get; set; }

        [ContentSerializer(Optional = true)]
        public string StyleClass { get; set; }

        [ContentSerializer(CollectionItemName = "Style")]
        public List<ControlStyleData> Styles { get; set; }
    }

    public class ControlStyleData
    {
        public string Name { get; set; }

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
        public string Color { get; set; } = "FFFFFFFF";

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
            var color = Microsoft.Xna.Framework.Color.White;

            uint colorPacked = 0;
            if(uint.TryParse(Color, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out colorPacked))
            {
                byte[] bytes = BitConverter.GetBytes(colorPacked);

                color = new Color(bytes[0], bytes[1], bytes[2], bytes[3]);
            }
            
            Texture2D texture = null;
            if (!string.IsNullOrWhiteSpace(Texture))
                texture = content.Load<Texture2D>(Texture);

            return new Brush(
                    color,
                    texture,
                    Margin,
                    ImageSize,
                    BrushType
                );
        }
    }
}
