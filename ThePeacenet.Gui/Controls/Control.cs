using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using SpriteFontPlus;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.TextLayout;

namespace ThePeacenet.Gui.Controls
{
    public abstract class Control : Element<Control>
    {
        private Thickness _clipPadding;
        private Thickness _padding;
        private bool _isVisible = true;
        private Thickness _margin;
        private HorizontalAlignment _hAlign = HorizontalAlignment.Stretch;
        private VerticalAlignment _vAlign = VerticalAlignment.Stretch;
        private HorizontalAlignment _hTextAlign = HorizontalAlignment.Centre;
        private VerticalAlignment _vTextAlign = VerticalAlignment.Centre;
        private bool _isLayoutRequired = true;

        private Screen _screen = null;

        public bool FocusOnClick { get; set; } = true;

        public abstract IEnumerable<Control> Children { get; }

        public Screen Screen { get => (_screen == null) ? Parent?.Screen : _screen; internal set => _screen = value; }

        protected sealed override void OnPropertyChanged(string propertyName)
        {
            base.OnPropertyChanged(propertyName);
            if(Screen != null)
            {
                Screen.ForceLayout();
            }
            IsLayoutRequired = true;
        }

        public Thickness ClipPadding
        {
            get => _clipPadding;
            set
            {
                if(_clipPadding != value)
                {
                    _clipPadding = value;
                    OnPropertyChanged(nameof(ClipPadding));
                }
            }
        }

        public Thickness Margin
        {
            get => _margin;
            set
            {
                if (_margin != value)
                {
                    _margin = value;
                    OnPropertyChanged(nameof(Margin));
                }
            }
        }

        public Thickness Padding
        {
            get => _padding;
            set
            {
                if (_padding != value)
                {
                    _padding = value;
                    OnPropertyChanged(nameof(Padding));
                }
            }
        }

        
        public bool IsVisible
        {
            get => _isVisible;
            set
            {
                if(_isVisible != value)
                {
                    _isVisible = value;
                    OnPropertyChanged(nameof(IsVisible));
                }
            }
        }

        public bool IsLayoutRequired { get => _isLayoutRequired || Children.Any(x=>x.IsLayoutRequired); set => _isLayoutRequired = value; }

        public HorizontalAlignment HorizontalAlignment
        {
            get => _hAlign;
            set
            {
                if(_hAlign != value)
                {
                    _hAlign = value;
                    OnPropertyChanged(nameof(HorizontalAlignment));
                }
            }
        }

        public HorizontalAlignment HorizontalTextAlignment
        {
            get => _hTextAlign;
            set
            {
                if (_hTextAlign != value)
                {
                    _hTextAlign = value;
                    OnPropertyChanged(nameof(HorizontalTextAlignment));
                }
            }
        }

        public VerticalAlignment VerticalAlignment
        {
            get => _vAlign;
            set
            {
                if (_vAlign != value)
                {
                    _vAlign = value;
                    OnPropertyChanged(nameof(VerticalAlignment));
                }
            }
        }

        public VerticalAlignment VerticalTextAlignment
        {
            get => _vTextAlign;
            set
            {
                if (_vTextAlign != value)
                {
                    _vTextAlign = value;
                    OnPropertyChanged(nameof(VerticalTextAlignment));
                }
            }
        }


        public Guid Id { get; private set; } = Guid.NewGuid();
        public Color TextColor { get; set; } = Color.White;
        public Vector2 TextOffset { get; set; }
        public DynamicSpriteFont Font { get; set; }

        public Rectangle ClippingRectangle
        {
            get
            {
                var r = BoundingRectangle;
                return new Rectangle(r.Left + ClipPadding.Left, r.Top + ClipPadding.Top, r.Width - ClipPadding.Width, r.Height - ClipPadding.Height);
            }
        }

        public Rectangle ContentRectangle
        {
            get
            {
                var r = BoundingRectangle;
                return new Rectangle(r.Left + Padding.Left, r.Top + Padding.Top, r.Width - Padding.Width, r.Height - Padding.Height);
            }
        }

        public void RemoveFromParent()
        {
            if(Parent is ItemsControl items)
            {
                items.Items.Remove(this);
            }
            else if(Parent is ContentControl content)
            {
                content.Content = null;
            }
            Parent = null;
        }

        public virtual void Update(IGuiContext context, float deltaSeconds) { }

        public virtual void InvalidateMeasure() { }

        public abstract Size2 GetContentSize(IGuiContext context);

        public virtual Size2 CalculateActualSize(IGuiContext context)
        {
            var fixedSize = Size;
            var desiredSize = GetContentSize(context) + Margin.Size + Padding.Size;

            if (desiredSize.Width < MinWidth)
                desiredSize.Width = MinWidth;

            if (desiredSize.Height < MinHeight)
                desiredSize.Height = MinHeight;

            if (desiredSize.Width > MaxWidth)
                desiredSize.Width = MaxWidth;

            if (desiredSize.Height > MaxWidth)
                desiredSize.Height = MaxHeight;

            var width = fixedSize.Width == 0 ? desiredSize.Width : fixedSize.Width;
            var height = fixedSize.Height == 0 ? desiredSize.Height : fixedSize.Height;
            IsLayoutRequired = false;
            return new Size2(width, height);
        }

        public T FindControl<T>(string name) where T: Control
        {
            return Screen.FindControl<T>(this, name);
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            renderer.DrawBrush(BoundingRectangle, BackgroundBrush);
            renderer.DrawBrush(BoundingRectangle, BorderBrush);
        }

        private ControlStyle _hoverStyle;
        public ControlStyle HoverStyle
        {
            get => _hoverStyle;
            set
            {
                if (_hoverStyle != value)
                {
                    _hoverStyle = value;
                    HoverStyle?.ApplyIf(this, _isHovered);
                }
            }
        }

        private ControlStyle _disabledStyle;
        public ControlStyle DisabledStyle
        {
            get => _disabledStyle;
            set
            {
                _disabledStyle = value;
                DisabledStyle?.ApplyIf(this, !_isEnabled);
            }
        }

        public bool IsFocused { get; set; }

        public virtual void OnScrolled(int delta) { }

        public virtual bool OnKeyTyped(IGuiContext context, KeyboardEventArgs args) { return true; }
        public virtual bool OnKeyPressed(IGuiContext context, KeyboardEventArgs args) { return true; }

        public virtual bool OnFocus(IGuiContext context) { return true; }
        public virtual bool OnUnfocus(IGuiContext context) { return true; }

        public virtual bool OnPointerDown(IGuiContext context, PointerEventArgs args) { return true; }
        public virtual bool OnPointerMove(IGuiContext context, PointerEventArgs args) { return true; }
        public virtual bool OnPointerUp(IGuiContext context, PointerEventArgs args) { return true; }

        public virtual bool OnPointerEnter(IGuiContext context, PointerEventArgs args)
        {
            if (IsEnabled && !IsHovered)
                IsHovered = true;

            return true;
        }

        public virtual bool OnPointerLeave(IGuiContext context, PointerEventArgs args)
        {
            if (IsEnabled && IsHovered)
                IsHovered = false;

            return true;
        }

        private bool _isEnabled = true;
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    DisabledStyle?.ApplyIf(this, !_isEnabled);
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        private bool _isHovered;

        [EditorBrowsable(EditorBrowsableState.Never)]
        public bool IsHovered
        {
            get => _isHovered;
            private set
            {
                if (_isHovered != value)
                {
                    _isHovered = value;
                    HoverStyle?.ApplyIf(this, _isHovered);
                    OnPropertyChanged(nameof(IsHovered));
                }
            }
        }

        public bool HasParent(Control control)
        {
            return Parent != null && (Parent == control || Parent.HasParent(control));
        }

        public virtual bool Contains(IGuiContext context, Point point)
        {
            return BoundingRectangle.Contains(point);
        }

        protected TextInfo GetTextInfo(IGuiContext context, string text, Rectangle targetRectangle, HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment)
        {
            var font = Font ?? context.DefaultFont;
            var wrapped = TextMeasure.WrapText(font, text ?? string.Empty, targetRectangle, WrapMode.WordWrap);
            var textSize = LayoutHelper.MeasureString(font, wrapped);
            var destinationRectangle = LayoutHelper.AlignRectangle(horizontalAlignment, verticalAlignment, textSize, targetRectangle);
            var textPosition = destinationRectangle.Location.ToVector2();
            var textInfo = new TextInfo(wrapped, font, textPosition, textSize, TextColor, targetRectangle);
            return textInfo;
        }

        public struct TextInfo
        {
            public TextInfo(string text, DynamicSpriteFont font, Vector2 position, Size2 size, Color color, Rectangle? clippingRectangle)
            {
                Text = text ?? string.Empty;
                Font = font;
                Size = size;
                Color = color;
                ClippingRectangle = clippingRectangle;
                Position = position;
            }

            public string Text;
            public DynamicSpriteFont Font;
            public Size2 Size;
            public Color Color;
            public Rectangle? ClippingRectangle;
            public Vector2 Position;
        }

        public Dictionary<string, object> AttachedProperties { get; } = new Dictionary<string, object>();

        public bool HasFocus => (IsFocused || Children.Any(x => x.HasFocus));
        public int ZOrder { get; set; } = 0;

        private string _styleClass = "";
        public string StyleClass
        {
            get => _styleClass;
            set
            {
                if(_styleClass != value)
                {
                    _styleClass = value;
                    OnPropertyChanged(nameof(StyleClass));
                }
            }
        }

        private GuiSkin _skin = null;

        public void ApplySkin(ContentManager content, GuiSkin skn)
        {
            if (_skin == skn) return;

            var type = this.GetType();

            SkinElement element = skn.Elements.FirstOrDefault(x => x.Control == this.GetType().FullName);
            if (element == null) return;

            var style = element.Styles.FirstOrDefault(x => x.Name == this.StyleClass);
            if (style == null)
                style = element.Styles.FirstOrDefault(x => string.IsNullOrWhiteSpace(x.Name));

            if (style == null) return;

            foreach(var state in style.States)
            {
                var styleProp = type.GetProperty(state.Name);

                if(state.Name != "Default" && !typeof(ControlStyle).IsAssignableFrom(styleProp.PropertyType))
                    throw new InvalidOperationException(string.Format("Cannot apply skin style to {0}: {0}.{1} is not a Control Style property.", type.FullName, styleProp.Name));

                var styleValue = new ControlStyle(type);
                
                foreach(var brushProperty in state.Brushes)
                {
                    var prop = type.GetProperty(brushProperty.Name);

                    if (prop.PropertyType != typeof(Brush))
                        throw new InvalidOperationException("Cannot apply brush to a non-brush property.");

                    if (!styleValue.ContainsKey(prop.Name))
                        styleValue.Add(prop.Name, brushProperty.CreateBrush(content));
                }

                foreach(var fontProperty in state.Fonts)
                {
                    var prop = type.GetProperty(fontProperty.Property);

                    if (prop.PropertyType != typeof(DynamicSpriteFont))
                        throw new InvalidOperationException("Cannot apply font to a non-font property.");

                    var font = content.LoadFont(fontProperty.Path);
                    font.Size = fontProperty.Size;
                    styleValue.Add(prop.Name, font);
                }

                foreach(var property in state.Properties)
                {
                    var prop = type.GetProperty(property.Name);

                    if (!prop.PropertyType.IsAssignableFrom(property.Value.GetType()))
                        throw new InvalidOperationException("Cannot apply skin property to control property.");

                    styleValue.Add(prop.Name, property.Value);

                }

                if (styleProp == null)
                {
                    styleValue.ApplyIf(this, true);
                }
                else
                {
                    styleProp.SetValue(this, styleValue);
                }
            }

            _skin = skn;
        }

        public bool IsPartOf<T>()
        {
            if (this is T) return true;

            var parent = Parent;

            while(parent != null)
            {
                if (parent is T) return true;
                parent = parent.Parent;
            }

            return false;
        }

        public object GetAttachedProperty(string name)
        {
            return AttachedProperties.TryGetValue(name, out var value) ? value : null;
        }

        public void SetAttachedProperty(string name, object value)
        {
            AttachedProperties[name] = value;
            OnPropertyChanged(name);
        }

        public virtual Type GetAttachedPropertyType(string propertyName)
        {
            return null;
        }
    }
}
