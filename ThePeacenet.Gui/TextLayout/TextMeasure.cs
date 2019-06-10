using System;
using SpriteFontPlus;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using System.Text;

namespace ThePeacenet.Gui.TextLayout
{
    public static class TextMeasure
    {
        private static string WrapInternal(string text, Rectangle targetRectangle, WrapMode wrapMode, Func<string, Size2> measureFunc)
        {
            if (wrapMode == WrapMode.None)
                return text;

            if(wrapMode == WrapMode.LetterWrap)
            {
                string ret = "";
                float w = 0;
                foreach(char c in text)
                {
                    var measure = measureFunc(c.ToString());
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

            if (wrapMode == WrapMode.WordWrap)
            {
                string[] words = text.Split(' ');
                StringBuilder sb = new StringBuilder();
                float lineWidth = 0f;
                float spaceWidth = measureFunc(" ").Width;

                foreach (string word in words)
                {
                    var size = measureFunc(word);

                    if (word.Contains("\r"))
                    {
                        lineWidth = 0f;
                        sb.Append("\r \r");
                    }

                    if (lineWidth + size.Width <= targetRectangle.Width)
                    {
                        sb.Append(word + " ");
                        lineWidth += size.Width + spaceWidth;
                    }

                    else
                    {
                        if (size.Width > targetRectangle.Width)
                        {
                            if  (sb.ToString() == " ")
                            {
                                sb.Append(WrapInternal(word.Insert(word.Length / 2, " ") + " ", targetRectangle, wrapMode, measureFunc));
                            }
                            else
                            {
                                sb.Append("\n" + WrapInternal(word.Insert(word.Length / 2, " ") + " ", targetRectangle, wrapMode, measureFunc));
                            }
                        }
                        else
                        {
                            sb.Append("\n" + word + " ");
                            lineWidth = size.Width + spaceWidth;
                        }
                    }
                }

                return sb.ToString();
            }
            return text;
        }

        public static string WrapText(DynamicSpriteFont font, string text, Rectangle targetRectangle, WrapMode wrapMode)
        {
            return WrapInternal(text, targetRectangle, wrapMode, (c) =>
            {
                var measure = font.MeasureString(c);
                return new Size2(measure.X, measure.Y);
            });
        }

        public static string WrapText(SpriteFont font, string text, Rectangle targetRectangle, WrapMode wrapMode)
        {
            return WrapInternal(text, targetRectangle, wrapMode, (c) =>
            {
                var measure = font.MeasureString(c);
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
