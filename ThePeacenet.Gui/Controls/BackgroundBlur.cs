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
            // Grab the on-screen portion of the control's bounding box
            var visibleRect = Rectangle.Intersect(BoundingRectangle, renderer.BoundingRectangle);

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

            // Use the blur effect.
            renderer.SetRenderEffect(RenderEffect.Blur);
            
            // This computes the sample weights and sample offsets for the first Gaussian pass.
            // The first pass is a horizontal blur which we render to the back buffer.
            renderer.SetGaussianParameters(1.0f / visibleRect.Width, 0, BlurAmount);

            // And now we draw it!  TODO: Use a blur effect.
            renderer.DrawBrush(visibleRect, new Brush(Color.White, _blurTexture, new Thickness(0), Size2.Empty, BrushType.Image));

            // Now we need to end the batch operation.  This causes the above sprite to be blitted into the back buffer.  We need
            // to do this so we can grab the now-blurred back buffer pixel data.
            renderer.End();

            // Grab the blurred back buffer data.
            context.GetBackBufferData(visibleRect, _pixelData);

            // Blit it into our blur texture so we can apply a second blur pass on it.
            _blurTexture.SetData<Color>(_pixelData);

            // Begin another draw call with the blur effect still enabled.
            // We're also going to compute the sample weights and sample offsets for a vertical
            // Gaussian blur.
            renderer.Begin();
            renderer.SetGaussianParameters(0, 1.0f / visibleRect.Height, BlurAmount);
            
            // And now we render the texture to the screen! Again.  Now it's vertically blurred.
            renderer.DrawBrush(visibleRect, new Brush(Color.White, _blurTexture, new Thickness(0), Size2.Empty, BrushType.Image));
            
            // And stop using the blur effect.
            renderer.SetRenderEffect(RenderEffect.None);

            // Render our content.
            base.Draw(context, renderer, deltaSeconds);
        }
    }
}
