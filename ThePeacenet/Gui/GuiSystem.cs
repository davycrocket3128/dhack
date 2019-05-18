using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using MonoGame.Extended.ViewportAdapters;
using ThePeacenet.Core;

namespace ThePeacenet.Gui
{
    public class GuiSystem : IRenderable, IRectangular
    {
        private ViewportAdapter _viewportAdapter = null;

        public GuiSystem(ViewportAdapter viewportAdapter)
        {
            _viewportAdapter = viewportAdapter;

            
        }

        public Rectangle BoundingRectangle => _viewportAdapter.BoundingRectangle;

        public void Draw(GameTime gameTime)
        {
        }

        public void Update(GameTime gameTime)
        {
        }
    }
}
