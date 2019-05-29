using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using ThePeacenet.Backend.Gui;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Desktop
{
    public class Window : CompositeControl, IWindow
    {
        private Border _rootBorder = null;
        private DockPanel _nonClient = null;
        private StackPanel _titleButtons = null;
        private Image _iconImage = null;
        private Label _caption = null;
        private DragSurface _captionBorder = null;
        private Border _clientArea = null;
        private DockPanel _captionDock = null;
        private Button _closeButton = null;
        private Button _maximizeButton = null;
        private Button _minimizeButton = null;

        public Window()
        {
            _rootBorder = new Border
            {
                Content = _nonClient = new DockPanel
                {
                    LastChildFill = true
                }
            };

            _nonClient.Items.Add(_captionBorder = new DragSurface
            {
                Content = _captionDock = new DockPanel
                {
                    LastChildFill = true
                }
            });

            _nonClient.Items.Add(_clientArea = new Border());

            _captionDock.Items.Add(_iconImage = new Image
            {
                VerticalAlignment = VerticalAlignment.Centre
            });

            _captionDock.Items.Add(_titleButtons = new StackPanel
            {
                Orientation = Orientation.Horizontal,
                VerticalAlignment = VerticalAlignment.Centre
            });

            _captionDock.Items.Add(_caption = new Label
            {
                VerticalAlignment = VerticalAlignment.Centre,
                HorizontalTextAlignment = HorizontalAlignment.Centre,
                Content = "Window Title",
            });

            _titleButtons.Items.Add(_minimizeButton = new Button
            {

            });

            _titleButtons.Items.Add(_maximizeButton = new Button
            {

            });

            _titleButtons.Items.Add(_closeButton = new Button
            {

            });

            _captionBorder.SetAttachedProperty(DockPanel.DockProperty, Dock.Top);
            _iconImage.SetAttachedProperty(DockPanel.DockProperty, Dock.Left);
            _titleButtons.SetAttachedProperty(DockPanel.DockProperty, Dock.Right);

            _clientArea.MinWidth = 320;
            _clientArea.MinHeight = 200;

            _captionBorder.PointerDrag += TitleBarPointerDrag;
        }

        private void TitleBarPointerDrag(object sender, DragSurfaceEventArgs e)
        {
            this.Position += e.Delta;
        }

        public override Control Template => _rootBorder;

        public bool IsMaximized => throw new NotImplementedException();

        public string WindowTitle { get => _caption.Content.ToString(); set => _caption.Content = value; }
        public bool CanClose { get => _closeButton.IsEnabled; set => _closeButton.IsEnabled = value; }
        public bool CanMinimize { get => _minimizeButton.IsVisible; set => _minimizeButton.IsVisible = value; }
        public bool CanMazimize { get => _maximizeButton.IsVisible; set => _maximizeButton.IsVisible = value; }

        public Texture2D WindowIcon
        {
            get => _iconImage.BackgroundBrush.Texture;
            set
            {
                if (value != _iconImage.BackgroundBrush.Texture)
                {
                    _iconImage.BackgroundBrush = new Gui.Brush(value, 16);
                }
            }
        }

        private void ApplyWindowTheme(WindowTheme theme)
        {
            if (theme == null) return;

            _rootBorder.BackgroundBrush = theme.BackgroundBrush;
            _clientArea.Margin = theme.ClientBorderMargin;
            _caption.TextColor = theme.TitleTextColor;

            if(_iconImage.BackgroundBrush.BrushColor != theme.WindowIconColor || (int)_iconImage.BackgroundBrush.ImageSize.Height != theme.WindowIconSize)
            {
                var b = new Brush(_iconImage.BackgroundBrush.Texture, theme.WindowIconSize);
                b.BrushColor = theme.WindowIconColor;
                _iconImage.BackgroundBrush = b;
            }
        }

        public void Close()
        {
            throw new NotImplementedException();
        }

        public void Maximize()
        {
            throw new NotImplementedException();
        }

        public void Minimize()
        {
            throw new NotImplementedException();
        }

        public void Restore()
        {
            throw new NotImplementedException();
        }

        public Control Content { get => _clientArea.Content; set => _clientArea.Content = value; }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            ApplyWindowTheme(WindowTheme.Current);
            base.Update(context, deltaSeconds);
        }
    }
}
