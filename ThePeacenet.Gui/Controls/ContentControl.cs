﻿using Microsoft.Xna.Framework;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Gui.Controls
{
    public class ContentControl : Control
    {
        private bool _contentChanged = true;

        private object _content;
        public object Content
        {
            get => _content;
            set
            {
                if (_content != value)
                {
                    _content = value;
                    _contentChanged = true;
                }
            }
        }

        public TextAlignment TextAlign { get; set; } = TextAlignment.Left;

        public override IEnumerable<Control> Children
        {
            get
            {
                if (Content is Control control)
                    yield return control;
            }
        }

        public bool HasContent => Content != null;

        public override void InvalidateMeasure()
        {
            base.InvalidateMeasure();
            _contentChanged = true;
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            if (_content is Control control && _contentChanged)
            {
                control.Parent = this;
                control.ActualSize = ContentRectangle.Size;
                control.Position = new Point(Padding.Left, Padding.Top);
                control.InvalidateMeasure();
                _contentChanged = false;
            }
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            base.Draw(context, renderer, deltaSeconds);

            // No reason to render our content if it's a control, the game will do that for us :P
            if(Content is string text)
            {
                var textInfo = GetTextInfo(context, text, ContentRectangle, HorizontalTextAlignment, VerticalTextAlignment);

                if (!string.IsNullOrWhiteSpace(textInfo.Text))
                    renderer.DrawString(textInfo.Font, textInfo.Text, textInfo.Position + TextOffset, textInfo.Color, ContentRectangle, TextAlign);
                    //renderer.DrawText(textInfo.Font, textInfo.Text, textInfo.Position + TextOffset, textInfo.Color, textInfo.ClippingRectangle);
            }
        }

        public override Size2 GetContentSize(IGuiContext context)
        {
            if (Content == null)
            {
                return BackgroundBrush.ImageSize;
            }

            if (Content is Control control)
                return control.CalculateActualSize(context);

            var text = Content?.ToString();
            var font = Font ?? context.DefaultFont;
            return LayoutHelper.MeasureString(font, text ?? string.Empty);
        }
    }
}
