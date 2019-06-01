using Microsoft.Xna.Framework.Content;
using SpriteFontPlus;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui
{
    public static class ContentExtensions
    {
        public static DynamicSpriteFont LoadFont(this ContentManager content, string path)
        {
            return DynamicSpriteFont.FromTtf(content.Load<byte[]>(path), 16);
        }
    }
}
