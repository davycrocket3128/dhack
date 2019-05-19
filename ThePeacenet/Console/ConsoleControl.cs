using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework.Content;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using ThePeacenet.Backend;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.OS;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Console
{
    public sealed class ConsoleControl : Control
    {
        private ConsoleRenderer _consoleRenderer = null;

        public ConsoleControl(ContentManager content, IUserLand owner)
        {
            _consoleRenderer = new ConsoleRenderer(content, owner);
        }

        public IConsoleContext Console => _consoleRenderer;

        public override IEnumerable<Control> Children => Enumerable.Empty<Control>();

        public override Size2 GetContentSize(IGuiContext context)
        {
            return new Size2();
        }

        public override void OnScrolled(int delta)
        {
            _consoleRenderer.HandleScrollEvent(delta);
            base.OnScrolled(delta);
        }

        public override bool OnKeyPressed(IGuiContext context, KeyboardEventArgs args)
        {
            _consoleRenderer.HandleKeyPressed(args);
            return base.OnKeyPressed(context, args);
        }

        public override bool OnKeyTyped(IGuiContext context, KeyboardEventArgs args)
        {
            _consoleRenderer.HandleKeyTyped(args);
            return base.OnKeyTyped(context, args);
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            _consoleRenderer.Draw(renderer, deltaSeconds);
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            base.Update(context, deltaSeconds);
            _consoleRenderer.BoundingRectangle = this.ContentRectangle;
            _consoleRenderer.Update(deltaSeconds);
        }
    }
}
