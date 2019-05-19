using Microsoft.Xna.Framework;
using MonoGame.Extended;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui
{
    public class Binding
    {
        public Binding(object viewModel, string viewModelProperty, string viewProperty)
        {
            ViewModel = viewModel;
            ViewModelProperty = viewModelProperty;
            ViewProperty = viewProperty;
        }

        public object ViewModel { get; }
        public string ViewModelProperty { get; }
        public string ViewProperty { get; }
    }

    public abstract class Element
    {
        public string Name { get; set; }
        public Point Position { get; set; }
        public Point Origin { get; set; }
        
        public Brush BorderBrush { get; set; }

        private Brush _backgroundBrush;
        public Brush BackgroundBrush
        {
            get => _backgroundBrush;
            set => _backgroundBrush = value;
        }

        public List<Binding> Bindings { get; } = new List<Binding>();

        protected void OnPropertyChanged(string propertyName)
        {
            foreach (var binding in Bindings)
            {
                if (binding.ViewProperty == propertyName)
                {
                    var value = GetType()
                        .GetTypeInfo()
                        .GetDeclaredProperty(binding.ViewProperty)
                        .GetValue(this);

                    binding.ViewModel
                        .GetType()
                        .GetTypeInfo()
                        .GetDeclaredProperty(binding.ViewModelProperty)
                        .SetValue(binding.ViewModel, value);
                }
            }
        }

        private Size2 _size;
        public Size2 Size
        {
            get => _size;
            set
            {
                _size = value;
                OnSizeChanged();
            }
        }

        protected virtual void OnSizeChanged() { }

        public int MinWidth { get; set; }
        public int MinHeight { get; set; }
        public int MaxWidth { get; set; } = int.MaxValue;
        public int MaxHeight { get; set; } = int.MaxValue;

        public int Width
        {
            get => (int)Size.Width;
            set => Size = new Size2(value, Size.Height);
        }

        public int Height
        {
            get => (int)Size.Height;
            set => Size = new Size2(Size.Width, value);
        }

        public Size2 ActualSize { get; internal set; }

        public abstract void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds);
    }

    public abstract class Element<TParent> : Element, IRectangular
        where TParent : IRectangular
    {
        [EditorBrowsable(EditorBrowsableState.Never)]
        [JsonIgnore]
        public TParent Parent { get; internal set; }

        [EditorBrowsable(EditorBrowsableState.Never)]
        [JsonIgnore]
        public Rectangle BoundingRectangle
        {
            get
            {
                var offset = Point.Zero;

                if (Parent != null)
                    offset = Parent.BoundingRectangle.Location;

                return new Rectangle(offset + Position - new Point((int)ActualSize.Width * Origin.X, (int)ActualSize.Height * Origin.Y), new Point((int)ActualSize.Width, (int)ActualSize.Height));
            }
        }
    }
}
