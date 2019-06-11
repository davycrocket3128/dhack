using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MonoGame.Extended;
using Microsoft.Xna.Framework.Graphics;
using SpriteFontPlus;
using ThePeacenet.Gui.TextLayout;

namespace ThePeacenet.Gui
{
    public interface IGuiRenderer
    {
        void Begin();
        void DrawRectangle(Rectangle rectangle, Color color, int thickness = 1);
        void FillRectangle(Rectangle rectangle, Color color);
        void DrawBrush(Rectangle rectangle, Brush brush);
        void DrawString(SpriteFont font, string text, Vector2 position, Color color, float scale = 1);
        void DrawString(DynamicSpriteFont font, string text, Vector2 position, Color color, Rectangle? bounds, TextAlignment textAlignment = TextAlignment.Left);

        void FillRectangle(RectangleF rectangle, Color color);
        void DrawRectangle(RectangleF rectangle, Color color, int thickness = 1);

        void SetScissorRect(Rectangle rect);

        void End();

        void GetBackBufferData(Rectangle rect, byte[] data);
    }

    public class GuiSpriteBatchRenderer : IGuiRenderer
    {
        private readonly Func<Matrix> _getTransformMatrix;
        private SpriteBatch _spriteBatch = null;

        public SpriteSortMode SortMode { get; set; }
        public BlendState BlendState { get; set; } = BlendState.AlphaBlend;
        public SamplerState SamplerState { get; set; } = SamplerState.LinearClamp;
        public DepthStencilState DepthStencilState { get; set; } = DepthStencilState.Default;
        public RasterizerState RasterizerState { get; set; } = RasterizerState.CullNone;
        public Effect Effect { get; set; }

        public void GetBackBufferData(Rectangle rect, byte[] data)
        {
            _spriteBatch.GraphicsDevice.GetBackBufferData(rect, data, 0, data.Length);
        }

        public void SetScissorRect(Rectangle rect)
        {
            _spriteBatch.GraphicsDevice.ScissorRectangle = rect;
        }

        public GuiSpriteBatchRenderer(GraphicsDevice graphics, Func<Matrix> getTransformMatrix)
        {
            _getTransformMatrix = getTransformMatrix;
            _spriteBatch = new SpriteBatch(graphics);

            RasterizerState = new RasterizerState
            {
                ScissorTestEnable = true,
                CullMode = CullMode.None,
                FillMode = FillMode.Solid,
                MultiSampleAntiAlias = true
            };
        }

        public void Begin()
        {
            _spriteBatch.Begin(SortMode, BlendState, SamplerState, DepthStencilState, RasterizerState, Effect, _getTransformMatrix());
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
                        // fill in the box if we're not a border.
                        if (brush.BrushType == BrushType.Box)
                        {
                            _spriteBatch.Draw(brush.Texture,
                                new Rectangle(rectangle.X + brush.Margin.Left, rectangle.Y + brush.Margin.Top, rectangle.Width - (brush.Margin.Left + brush.Margin.Right), rectangle.Height - (brush.Margin.Top + brush.Margin.Bottom)),
                                new Rectangle(brush.Margin.Left, brush.Margin.Top, brush.Texture.Width - (brush.Margin.Left + brush.Margin.Right), brush.Texture.Height - (brush.Margin.Top + brush.Margin.Bottom)),
                                brush.BrushColor);
                        }

                        // Draw top-right area.
                        _spriteBatch.Draw(brush.Texture, 
                            new Rectangle(rectangle.X, rectangle.Y, brush.Margin.Left, brush.Margin.Top),
                            new Rectangle(0, 0, brush.Margin.Left, brush.Margin.Top),
                            brush.BrushColor);

                        // Draw top-middle area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.X + brush.Margin.Left, rectangle.Y, rectangle.Width - (brush.Margin.Left + brush.Margin.Right), brush.Margin.Top),
                            new Rectangle(brush.Margin.Left, 0, brush.Texture.Width - (brush.Margin.Left + brush.Margin.Right), brush.Margin.Top),
                            brush.BrushColor);

                        // Draw top-right area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.Right - brush.Margin.Right, rectangle.Y, brush.Margin.Right, brush.Margin.Top),
                            new Rectangle(brush.Texture.Width - brush.Margin.Right, 0, brush.Margin.Right, brush.Margin.Top),
                            brush.BrushColor);

                        // Draw left area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.X, rectangle.Y + brush.Margin.Top, brush.Margin.Left, rectangle.Height - (brush.Margin.Top + brush.Margin.Bottom)),
                            new Rectangle(0, brush.Margin.Top, brush.Margin.Left, brush.Texture.Height - (brush.Margin.Top + brush.Margin.Bottom)),
                            brush.BrushColor);

                        // Draw right area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.Right - brush.Margin.Right, rectangle.Y + brush.Margin.Top, brush.Margin.Right, rectangle.Height - (brush.Margin.Top + brush.Margin.Bottom)),
                            new Rectangle(brush.Texture.Width - brush.Margin.Right, brush.Margin.Top, brush.Margin.Right, brush.Texture.Height - (brush.Margin.Top + brush.Margin.Bottom)),
                            brush.BrushColor);

                        // Draw bottom-right area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.X, rectangle.Bottom - brush.Margin.Bottom, brush.Margin.Left, brush.Margin.Bottom),
                            new Rectangle(0, brush.Texture.Height - brush.Margin.Bottom, brush.Margin.Left, brush.Margin.Bottom),
                            brush.BrushColor);

                        // Draw bottom-middle area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.X + brush.Margin.Left, rectangle.Bottom - brush.Margin.Bottom, rectangle.Width - (brush.Margin.Left + brush.Margin.Right), brush.Margin.Bottom),
                            new Rectangle(brush.Margin.Left, brush.Texture.Height - brush.Margin.Bottom, brush.Texture.Width - (brush.Margin.Left + brush.Margin.Right), brush.Margin.Bottom),
                            brush.BrushColor);

                        // Draw bottom-right area.
                        _spriteBatch.Draw(brush.Texture,
                            new Rectangle(rectangle.Right - brush.Margin.Right, rectangle.Bottom - brush.Margin.Bottom, brush.Margin.Right, brush.Margin.Bottom),
                            new Rectangle(brush.Texture.Width - brush.Margin.Right, brush.Texture.Height - brush.Margin.Bottom, brush.Margin.Right, brush.Margin.Bottom),
                            brush.BrushColor);


                    }
                    else
                    {
                        if (brush.BrushType == BrushType.Box)
                        {
                            FillRectangle(rectangle, brush.BrushColor);
                        }
                        else
                        {
                            _spriteBatch.FillRectangle(new Rectangle(rectangle.X, rectangle.Y, brush.Margin.Left, rectangle.Height), brush.BrushColor);
                            _spriteBatch.FillRectangle(new Rectangle(rectangle.X + brush.Margin.Left, rectangle.Y, rectangle.Width - brush.Margin.Width, brush.Margin.Bottom), brush.BrushColor);
                            _spriteBatch.FillRectangle(new Rectangle(rectangle.Right - brush.Margin.Right, rectangle.Y, brush.Margin.Left, rectangle.Height), brush.BrushColor);
                            _spriteBatch.FillRectangle(new Rectangle(rectangle.X + brush.Margin.Left, rectangle.Bottom - brush.Margin.Bottom, rectangle.Width - brush.Margin.Width, brush.Margin.Height), brush.BrushColor);

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

        public void DrawString(DynamicSpriteFont font, string text, Vector2 position, Color color, Rectangle? bounds, TextAlignment textAlign = TextAlignment.Left)
        {
            // Split the text into individual lines.
            var lines = TextMeasure.SplitLines(text);

            // The maximum width allotted for text.
            float maxWidth = 0;

            // Were we given a bounding box?
            if(bounds.HasValue)
            {
                // Maximum width becomes the width of the bounding box since text can't render outside of it.
                maxWidth = (float)bounds?.Width;
            }
            else
            {
                // Max width becomes the width of the widest line of text in the string.
                maxWidth = lines.Select(x => font.MeasureString(x.Trim()).X).Max();
            }

            // Where the line will render on-screen.
            float lineOffset = 0;

            // Go through every single line.
            foreach (var line in lines)
            {
                // Measure the line so we know how big it is and can calculate where to place it.
                var measure = font.MeasureString(line.Trim());

                // Where will the line go horizontally? Default is left-aligned so we'll place it at the requested x position.
                float x = position.X;

                if (textAlign == TextAlignment.Centre)
                    x += (maxWidth - measure.X) / 2;

                if (textAlign == TextAlignment.Right)
                    x += maxWidth - measure.X;

                font.DrawString(_spriteBatch, line, new Vector2(x, position.Y + lineOffset), color);
                lineOffset += measure.Y;
            }
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
