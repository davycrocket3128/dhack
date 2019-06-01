using Microsoft.Xna.Framework;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using SpriteFontPlus;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public abstract class Control : Element<Control>
    {
        public abstract IEnumerable<Control> Children { get; }
        public Thickness ClipPadding { get; set; }
        public Thickness Margin { get; set; }
        public Thickness Padding { get; set; }
        public bool IsVisible { get; set; } = true;
        public bool IsLayoutRequired { get; set; }
        public HorizontalAlignment HorizontalAlignment { get; set; } = HorizontalAlignment.Stretch;
        public VerticalAlignment VerticalAlignment { get; set; } = VerticalAlignment.Stretch;
        public HorizontalAlignment HorizontalTextAlignment { get; set; } = HorizontalAlignment.Centre;
        public VerticalAlignment VerticalTextAlignment { get; set; } = VerticalAlignment.Centre;
        public Guid Id { get; private set; } = Guid.NewGuid();
        public Color TextColor { get; set; } = Color.White;
        public Vector2 TextOffset { get; set; }
        public DynamicSpriteFont Font { get; set; }

        public Rectangle ClippingRectangle
        {
            get
            {
                var r = BoundingRectangle;
                return new Rectangle(r.Left + ClipPadding.Left, r.Top + ClipPadding.Top, r.Width - ClipPadding.Right, r.Height - ClipPadding.Bottom);
            }
        }

        public Rectangle ContentRectangle
        {
            get
            {
                var r = BoundingRectangle;
                return new Rectangle(r.Left + Padding.Left, r.Top + Padding.Top, r.Width - Padding.Right, r.Height - Padding.Bottom);
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
            return new Size2(width, height);
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

        private bool _isEnabled;
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    DisabledStyle?.ApplyIf(this, !_isEnabled);
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
            var textSize = (Size2)font.MeasureString(text ?? string.Empty);
            var destinationRectangle = LayoutHelper.AlignRectangle(horizontalAlignment, verticalAlignment, textSize, targetRectangle);
            var textPosition = destinationRectangle.Location.ToVector2();
            var textInfo = new TextInfo(text, font, textPosition, textSize, TextColor, targetRectangle);
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

        public object GetAttachedProperty(string name)
        {
            return AttachedProperties.TryGetValue(name, out var value) ? value : null;
        }

        public void SetAttachedProperty(string name, object value)
        {
            AttachedProperties[name] = value;
        }

        public virtual Type GetAttachedPropertyType(string propertyName)
        {
            return null;
        }
    }
}
