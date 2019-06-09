using Microsoft.Xna.Framework;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.Controls;
using ThePeacenet.Gui.Windowing;

namespace ThePeacenet.Gui
{
    public abstract class Screen : Element<GuiSystem>, IDisposable
    {
        private List<Window> _windows = new List<Window>();

        public IEnumerable<Window> Windows => _windows.OrderBy(x=>x.ZOrder);

        public virtual void Dispose()
        {
        }

        public GuiSkin Skin { get; set; } = null;

        private Control _content;
        
        internal void ForceLayout()
        {
            _isLayoutRequired = true;
        }

        public Control Content
        {
            get { return _content; }
            set
            {
                if (_content != value)
                {
                    _content = value;
                    _content.Screen = this;
                    _isLayoutRequired = true;
                }
            }
        }

        public void ShowWindow(Window window)
        {
            if(_windows.Contains(window))
            {
                window.IsVisible = true;
                return;
            }

            window.Screen = this;
            window.IsLayoutRequired = true;
            _windows.Add(window);

            BringToFront(window);

            _isLayoutRequired = true;
        }

        /*
        public float Width { get; private set; }
        public float Height { get; private set; }
        public Size2 Size => new Size2(Width, Height);
        */
        public bool IsVisible { get; set; } = true;
        

        private bool _isLayoutRequired;
        public bool IsLayoutRequired => _isLayoutRequired || Content.IsLayoutRequired || _windows.Any(x => x.IsLayoutRequired);

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

        public void BringToFront(Window window)
        {
            if(_windows.Contains(window))
            {
                int i = 0;
                foreach(var win in _windows.OrderBy(x=>x.ZOrder))
                {
                    if (win != window)
                    {
                        win.ZOrder = i;
                    }
                    i++;
                }

                window.ZOrder = i;
            }
        }

        public void Layout(IGuiContext context, Rectangle rectangle)
        {
            Width = rectangle.Width;
            Height = rectangle.Height;

            LayoutHelper.PlaceControl(context, Content, rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height);

            _isLayoutRequired = false;
            Content.IsLayoutRequired = false;

            foreach (var window in _windows)
            {
                if (window.IsLayoutRequired)
                {
                    var actualSize = window.CalculateActualSize(context);

                    if (window.JustOpened == true)
                    {
                        LayoutHelper.PlaceControl(context, window, (rectangle.Width - actualSize.Width) / 2, (rectangle.Height - actualSize.Height) / 2, actualSize.Width, actualSize.Height);
                        window.JustOpened = false;
                    }
                    else
                    {
                        LayoutHelper.PlaceControl(context, window, window.Position.X, window.Position.Y, actualSize.Width, actualSize.Height);
                    }

                    window.IsLayoutRequired = false;
                }

            }
        }

        public sealed override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }

        internal void RemoveWindow(Window window)
        {
            _windows.Remove(window);
        }
    }
}
