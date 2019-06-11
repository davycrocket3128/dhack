using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using ThePeacenet.Gui;

namespace ThePeacenet.Gui.Controls
{
    public class BackgroundBlur : ContentControl
    {
        private Color[] _pixelData = null;
        private Texture2D _blurTexture = null;

        public float BlurAmount { get; set; } = 3;

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            renderer.FillRectangle(BoundingRectangle, Color.Black * 0.25f);

            // Grab the on-screen portion of the control's bounding box
            var visibleRect = Rectangle.Intersect(BoundingRectangle, renderer.BoundingRectangle);

            // Use the blur effect.
            renderer.SetRenderEffect(RenderEffect.Blur);
            renderer.SetGaussianParameters(1.0f / visibleRect.Width, 1.0f / visibleRect.Height, BlurAmount);

            // If there's nothing visible, STOP RIGHT NOW or the game'll crash.
            if (visibleRect.Width * visibleRect.Height == 0) return;

            // If the width and height of the visible rectangle doesn't match the size of the pixel buffer/texture we'll
            // reallocate them so they're the same size.
            if(_pixelData == null || _pixelData.Length != (visibleRect.Width * visibleRect.Height))
            {
                _pixelData = new Color[visibleRect.Width * visibleRect.Height];
                if (_blurTexture != null)
                    _blurTexture.Dispose();
                _blurTexture = context.CreateTexture(visibleRect.Width, visibleRect.Height);
            }

            // Grab the back buffer data in the area of the screen where we are positioned.  A.K.A, the content that's
            // behind the control.
            context.GetBackBufferData(visibleRect, _pixelData);

            // Put the data in our texture so we can draw it as a sprite.
            _blurTexture.SetData<Color>(_pixelData);

            // And now we draw it!  TODO: Use a blur effect.
            renderer.DrawBrush(visibleRect, new Brush(Color.White, _blurTexture, new Thickness(0), Size2.Empty, BrushType.Image));

            // And stop using the blur effect.
            renderer.SetRenderEffect(RenderEffect.None);

            // Render our content.
            base.Draw(context, renderer, deltaSeconds);
        }
    }
}
