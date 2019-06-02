using Microsoft.Xna.Framework;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Gui
{
    public abstract class Screen : Element<GuiSystem>, IDisposable
    {
        public virtual void Dispose()
        {
        }

        public GuiSkin Skin { get; set; } = null;

        private Control _content;
        
        public Control Content
        {
            get { return _content; }
            set
            {
                if (_content != value)
                {
                    _content = value;
                    _isLayoutRequired = true;
                }
            }
        }

        /*
        public float Width { get; private set; }
        public float Height { get; private set; }
        public Size2 Size => new Size2(Width, Height);
        */
        public bool IsVisible { get; set; } = true;
        

        private bool _isLayoutRequired;
        public bool IsLayoutRequired => _isLayoutRequired || Content.IsLayoutRequired;

        public virtual void Update(GameTime gameTime)
        {

        }

        public void Show()
        {
            IsVisible = true;
        }

        public void Hide()
        {
            IsVisible = false;
        }

        public T FindControl<T>(string name)
            where T : Control
        {
            return FindControl<T>(Content, name);
        }

        public static T FindControl<T>(Control rootControl, string name)
            where T : Control
        {
            if (rootControl.Name == name)
                return rootControl as T;

            foreach (var childControl in rootControl.Children)
            {
                var control = FindControl<T>(childControl, name);

                if (control != null)
                    return control;
            }

            return null;
        }

        public void Layout(IGuiContext context, Rectangle rectangle)
        {
            Width = rectangle.Width;
            Height = rectangle.Height;

            LayoutHelper.PlaceControl(context, Content, rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height);

            _isLayoutRequired = false;
            Content.IsLayoutRequired = false;
        }
    }
}
