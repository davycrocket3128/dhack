using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MonoGame.Extended;
using Microsoft.Xna.Framework.Graphics;
using SpriteFontPlus;

namespace ThePeacenet.Gui
{
    public interface IGuiRenderer
    {
        void Begin();
        void DrawRectangle(Rectangle rectangle, Color color, int thickness = 1);
        void FillRectangle(Rectangle rectangle, Color color);
        void DrawBrush(Rectangle rectangle, Brush brush);
        void DrawString(SpriteFont font, string text, Vector2 position, Color color, float scale = 1);
        void DrawString(DynamicSpriteFont font, string text, Vector2 position, Color color);

        void FillRectangle(RectangleF rectangle, Color color);
        void DrawRectangle(RectangleF rectangle, Color color, int thickness = 1);

        void End();
    }

    public class GuiSpriteBatchRenderer : IGuiRenderer
    {
        private SpriteBatch _spriteBatch = null;

        public GuiSpriteBatchRenderer(GraphicsDevice graphics)
        {
            _spriteBatch = new SpriteBatch(graphics);
        }

        public void Begin()
        {
            _spriteBatch.Begin();
        }

        public void DrawBrush(Rectangle rectangle, Brush brush)
        {
            if (brush.BrushType == BrushType.None) return;

            switch(brush.BrushType)
            {
                case BrushType.Image:
                    if(brush.Texture != null)
                    {
                        _spriteBatch.Draw(brush.Texture, rectangle, brush.BrushColor);
                    }
                    else
                    {
                        FillRectangle(rectangle, brush.BrushColor);
                    }
                    break;
                case BrushType.Box:
                case BrushType.Border:
                    if (brush.Texture != null)
                    {
                        // draw top-left corner.
                        _spriteBatch.Draw(brush.Texture, new Rectangle(rectangle.X, rectangle.Y, brush.Margin.Left, brush.Margin.Top), new Rectangle(0, 0, brush.Margin.Left, brush.Margin.Top), brush.BrushColor);
                    }
                    else
                    {
                        if (brush.BrushType == BrushType.Box)
                        {
                            FillRectangle(rectangle, brush.BrushColor);
                        }
                        else
                        {
                            _spriteBatch.DrawLine(rectangle.X, rectangle.Y, rectangle.X, rectangle.Y + rectangle.Height, brush.BrushColor, brush.Margin.Left);
                            _spriteBatch.DrawLine(rectangle.X, rectangle.Y, rectangle.X + rectangle.Width, rectangle.Y, brush.BrushColor, brush.Margin.Top);
                            _spriteBatch.DrawLine(rectangle.X + rectangle.Width, rectangle.Y, rectangle.X + rectangle.Width, rectangle.Y + rectangle.Height, brush.BrushColor, brush.Margin.Right);
                            _spriteBatch.DrawLine(rectangle.X + rectangle.Width, rectangle.Y + rectangle.Height, rectangle.X, rectangle.Y + rectangle.Height, brush.BrushColor, brush.Margin.Bottom);
                        }
                    }
                    break;
            }
        }

        public void DrawRectangle(Rectangle rectangle, Color color, int thickness = 1)
        {
            _spriteBatch.DrawRectangle(rectangle, color, thickness);
        }

        public void DrawRectangle(RectangleF rectangle, Color color, int thickness = 1)
        {
            _spriteBatch.DrawRectangle(rectangle, color, thickness);
        }

        public void DrawString(SpriteFont font, string text, Vector2 position, Color color, float scale = 1)
        {
            _spriteBatch.DrawString(font, text, position, color, 0f, Vector2.Zero, scale, SpriteEffects.None, 0);
        }

        public void DrawString(DynamicSpriteFont font, string text, Vector2 position, Color color)
        {
            font.DrawString(_spriteBatch, text, position, color);
        }

        public void End()
        {
            _spriteBatch.End();
        }

        public void FillRectangle(Rectangle rectangle, Color color)
        {
            _spriteBatch.FillRectangle(rectangle, color);
        }

        public void FillRectangle(RectangleF rectangle, Color color)
        {
            _spriteBatch.FillRectangle(rectangle, color);
        }
    }
}
