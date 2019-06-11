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
    public enum RenderEffect
    {
        None,
        Blur
    }

    public interface IGuiRenderer
    {
        Rectangle BoundingRectangle { get; }

        void Begin();
        void DrawRectangle(Rectangle rectangle, Color color, int thickness = 1);
        void FillRectangle(Rectangle rectangle, Color color);
        void DrawBrush(Rectangle rectangle, Brush brush);
        void DrawString(SpriteFont font, string text, Vector2 position, Color color, float scale = 1);
        void DrawString(DynamicSpriteFont font, string text, Vector2 position, Color color, Rectangle? bounds, TextAlignment textAlignment = TextAlignment.Left);

        void SetRenderEffect(RenderEffect effect);

        void FillRectangle(RectangleF rectangle, Color color);
        void DrawRectangle(RectangleF rectangle, Color color, int thickness = 1);

        void SetGaussianParameters(float dx, float dy, float blurAmount);

        void SetScissorRect(Rectangle rect);

        void End();

        void GetBackBufferData<T>(Rectangle rect, T[] data) where T : struct;

        Texture2D CreateTexture(int width, int height);
    }

    public class GuiSpriteBatchRenderer : IGuiRenderer
    {
        public Rectangle BoundingRectangle => new Rectangle(0, 0, _spriteBatch.GraphicsDevice.PresentationParameters.BackBufferWidth, _spriteBatch.GraphicsDevice.PresentationParameters.BackBufferHeight);

        private readonly Func<Matrix> _getTransformMatrix;
        private SpriteBatch _spriteBatch = null;
        private bool _isBatching = false;

        public SpriteSortMode SortMode { get; set; }
        public BlendState BlendState { get; set; } = BlendState.AlphaBlend;
        public SamplerState SamplerState { get; set; } = SamplerState.LinearClamp;
        public DepthStencilState DepthStencilState { get; set; } = DepthStencilState.Default;
        public RasterizerState RasterizerState { get; set; } = RasterizerState.CullNone;
        public Effect Effect { get; set; }

        private Effect _blurEffect = null;

        /// "Borrowed" from https://github.com/CartBlanche/MonoGame-Samples/blob/master/BloomSample/BloomComponent.cs
        /// <summary>
        /// Computes sample weightings and texture coordinate offsets
        /// for one pass of a separable gaussian blur filter.
        /// </summary>
        public void SetGaussianParameters(float dx, float dy, float blurAmount)
        {
            // Look up the sample weight and offset effect parameters.
            EffectParameter weightsParameter, offsetsParameter;

            weightsParameter = _blurEffect.Parameters["SampleWeights"];
            offsetsParameter = _blurEffect.Parameters["SampleOffsets"];

            // Look up how many samples our gaussian blur effect supports.
            int sampleCount = weightsParameter.Elements.Count;

            // Create temporary arrays for computing our filter settings.
            float[] sampleWeights = new float[sampleCount];
            Vector2[] sampleOffsets = new Vector2[sampleCount];

            // The first sample always has a zero offset.
            sampleWeights[0] = ComputeGaussian(0, blurAmount);
            sampleOffsets[0] = new Vector2(0);

            // Maintain a sum of all the weighting values.
            float totalWeights = sampleWeights[0];

            // Add pairs of additional sample taps, positioned
            // along a line in both directions from the center.
            for (int i = 0; i < sampleCount / 2; i++)
            {
                // Store weights for the positive and negative taps.
                float weight = ComputeGaussian(i + 1, blurAmount);

                sampleWeights[i * 2 + 1] = weight;
                sampleWeights[i * 2 + 2] = weight;

                totalWeights += weight * 2;

                // To get the maximum amount of blurring from a limited number of
                // pixel shader samples, we take advantage of the bilinear filtering
                // hardware inside the texture fetch unit. If we position our texture
                // coordinates exactly halfway between two texels, the filtering unit
                // will average them for us, giving two samples for the price of one.
                // This allows us to step in units of two texels per sample, rather
                // than just one at a time. The 1.5 offset kicks things off by
                // positioning us nicely in between two texels.
                float sampleOffset = i * 2 + 1.5f;

                Vector2 delta = new Vector2(dx, dy) * sampleOffset;

                // Store texture coordinate offsets for the positive and negative taps.
                sampleOffsets[i * 2 + 1] = delta;
                sampleOffsets[i * 2 + 2] = -delta;
            }

            // Normalize the list of sample weightings, so they will always sum to one.
            for (int i = 0; i < sampleWeights.Length; i++)
            {
                sampleWeights[i] /= totalWeights;
            }

            // Tell the effect about our new filter settings.
            weightsParameter.SetValue(sampleWeights);
            offsetsParameter.SetValue(sampleOffsets);
        }

        /// "Borrowed" from https://github.com/CartBlanche/MonoGame-Samples/blob/master/BloomSample/BloomComponent.cs
        /// <summary>
        /// Evaluates a single point on the gaussian falloff curve.
        /// Used for setting up the blur filter weightings.
        /// </summary>
        private float ComputeGaussian(float n, float blurAmount)
        {
            float theta = blurAmount;

            return (float)((1.0 / Math.Sqrt(2 * Math.PI * theta)) *
                           Math.Exp(-(n * n) / (2 * theta * theta)));
        }

        public void SetRenderEffect(RenderEffect effect)
        {
            if (_isBatching) End();

            switch(effect)
            {
                case RenderEffect.Blur:
                    Effect = _blurEffect;
                    break;
                default:
                    Effect = null;
                    break;
            }

            Begin();
        }

        public Texture2D CreateTexture(int width, int height)
        {
            return new Texture2D(_spriteBatch.GraphicsDevice, width, height);
        }

        public void GetBackBufferData<T>(Rectangle rect, T[] data) where T: struct
        {
            _spriteBatch.GraphicsDevice.GetBackBufferData<T>(rect, data, 0, data.Length);
        }

        public void SetScissorRect(Rectangle rect)
        {
            _spriteBatch.GraphicsDevice.ScissorRectangle = rect;
        }

        public GuiSpriteBatchRenderer(GraphicsDevice graphics, Func<Matrix> getTransformMatrix, Effect blurEffect)
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

            _blurEffect = blurEffect;
        }

        public void Begin()
        {
            _isBatching = true;
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
            _isBatching = false;
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
