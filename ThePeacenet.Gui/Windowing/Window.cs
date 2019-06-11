using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using ThePeacenet.Backend;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Gui.Windowing
{
    public class Window : CompositeControl, IWindow
    {
        private readonly Border _rootBorder = null;
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
        private bool _shown = false;
        private bool _justOpened = true;
        private ITickable _guiHandler = null;

        public void SetGuiHandler(ITickable handler)
        {
            _guiHandler = handler;
        }

        public bool DrawContentBackground
        {
            get => _clientArea.StyleClass == "WindowContent";
            set => _clientArea.StyleClass = (value) ? "WindowContent" : "";
        }

        public event EventHandler Load;

        public Window() : base()
        {            
            _rootBorder = new Border
            {
                Content = _nonClient = new DockPanel
                {
                    LastChildFill = true
                },
                StyleClass = "WindowBorder"
            };

            _nonClient.Items.Add(_captionBorder = new DragSurface
            {
                Content = _captionDock = new DockPanel
                {
                    LastChildFill = true
                },
                StyleClass = "WindowTitlebar"
            });

            _nonClient.Items.Add(_clientArea = new Border
            {
                StyleClass = "WindowContent"
            });

            _captionDock.Items.Add(_iconImage = new Image
            {
                VerticalAlignment = VerticalAlignment.Centre,
                StyleClass = "WindowIcon"
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
                StyleClass = "TitleText"
            });

            _titleButtons.Items.Add(_minimizeButton = new Button
            {
                StyleClass = "WindowMinimize"
            });

            _titleButtons.Items.Add(_maximizeButton = new Button
            {
                StyleClass = "WindowMaximize"
            });

            _titleButtons.Items.Add(_closeButton = new Button
            {
                StyleClass = "WindowClose"
            });

            _captionBorder.SetAttachedProperty(DockPanel.DockProperty, Dock.Top);
            _iconImage.SetAttachedProperty(DockPanel.DockProperty, Dock.Left);
            _titleButtons.SetAttachedProperty(DockPanel.DockProperty, Dock.Right);

            _clientArea.MinWidth = 320;
            _clientArea.MinHeight = 200;

            _captionBorder.PointerDrag += TitleBarPointerDrag;

            _closeButton.Clicked += (o, a) => { Close(); };
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
                    _iconImage.BackgroundBrush = new Brush(value, 16);
                }
            }
        }

        public override bool OnPointerDown(IGuiContext context, PointerEventArgs args)
        {
            return base.OnPointerDown(context, args);
        }

        public void Close()
        {
            Screen.RemoveWindow(this);
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

        public Control Content { get => _clientArea.Content as Control; set => _clientArea.Content = value; }

        public bool JustOpened { get => _justOpened; internal set => _justOpened = value; }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            if(_shown == false)
            {
                _shown = true;
                Load?.Invoke(this, EventArgs.Empty);
            }

            if (_guiHandler != null) _guiHandler.Update(deltaSeconds);

            base.Update(context, deltaSeconds);
        }
    }
}
