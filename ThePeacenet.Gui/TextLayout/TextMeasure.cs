using System;
using SpriteFontPlus;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;

namespace ThePeacenet.Gui.TextLayout
{
    public static class TextMeasure
    {
        private static string WrapInternal(string text, Rectangle targetRectangle, WrapMode wrapMode, Func<char, Size2> measureFunc)
        {
            if (wrapMode == WrapMode.None)
                return text;

            if(wrapMode == WrapMode.LetterWrap)
            {
                string ret = "";
                float w = 0;
                foreach(char c in text)
                {
                    var measure = measureFunc(c);
                    if(w + measure.Width > targetRectangle.Width)
                    {
                        ret += Environment.NewLine;
                        w = 0;
                    }
                    else
                    {
                        w += measure.Width;
                    }
                    ret += c;
                }
                return ret;
            }

            return text;
        }

        public static string WrapText(DynamicSpriteFont font, string text, Rectangle targetRectangle, WrapMode wrapMode)
        {
            return WrapInternal(text, targetRectangle, wrapMode, (c) =>
            {
                var measure = font.MeasureString(c.ToString());
                return new Size2(measure.X, measure.Y);
            });
        }

        public static string WrapText(SpriteFont font, string text, Rectangle targetRectangle, WrapMode wrapMode)
        {
            return WrapInternal(text, targetRectangle, wrapMode, (c) =>
            {
                var measure = font.MeasureString(c.ToString());
                return new Size2(measure.X, measure.Y);
            });
        }
    }

    public enum WrapMode
    {
        None,
        LetterWrap,
        WordWrap
    }
}
