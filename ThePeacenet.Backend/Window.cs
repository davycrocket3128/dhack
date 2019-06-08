using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Gui;
using ThePeacenet.Backend.OS;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Backend
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
        private ContentManager _content = null;
        private readonly IUserLand _owner = null;
        private GuiHandler _guiHandler = null;
        private bool _shown = false;
        private bool _justOpened = false;

        public void SetGuiHandler(GuiHandler handler)
        {
            if (_guiHandler != null)
                throw new InvalidOperationException("The window has already been initialized.");

            _guiHandler = handler;
            _guiHandler.Initialize(this);
        }

        public IUserLand User => _owner;

        public event EventHandler Load;

        public Window(ContentManager content, IUserLand owner) : base()
        {
            _content = content;
            _owner = owner;

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

        public Window(ThePeacenet.Backend.AssetTypes.Program program, IUserLand owner) : this(program.Content, owner)
        {
            Build(program);
        }

        private void Build(Backend.AssetTypes.Program program)
        {
            // Set up the non-client first.
            this._minimizeButton.IsVisible = program.EnableMinimizeMaximize;
            this._maximizeButton.IsVisible = program.EnableMinimizeMaximize;
            this.WindowTitle = program.Name;
            this.WindowIcon = _content.Load<Texture2D>(program.LauncherIcon);

            // Builds the GUI for the window based on the compiled program.
            // This is a black box, I have no fucking clue what'll happen in this
            // delegate.
            _clientArea.Content = program.WindowBuilder(this, _content, _owner);

            // Build the first page.
            // BuildPage(program.Gui.Pages.First());
        }

        private Control BuildControl(ControlElement controlElement)
        {
            var type = Type.GetType(controlElement.Type, false);
            if(type == null)
            {
                foreach(var ass in AppDomain.CurrentDomain.GetAssemblies())
                {
                    type = ass.GetTypes().FirstOrDefault(x => x.FullName == controlElement.Type);
                    if (type != null) break;
                }
            }

            if (!typeof(Control).IsAssignableFrom(type))
                throw new InvalidOperationException(string.Format("Error building program GUI: Type {0} is not a control.", controlElement.Type));

            var control = (Control)Activator.CreateInstance(type, null);
            control.Name = controlElement.Name;

            var buildMethod = type.GetMethod("Build");

            if(buildMethod != null)
            {
                buildMethod.Invoke(control, new object[] { _content, _owner });
            }

            foreach(var property in controlElement.Properties)
            {
                var propInfo = type.GetProperty(property.Name);

                if(propInfo != null && propInfo.CanWrite)
                {
                    propInfo.SetValue(control, property.Value);
                }
                else
                {
                    control.SetAttachedProperty(property.Name, property.Value);
                }
            }

            // Bind all control events.
            foreach (var eventBind in controlElement.Events)
            {
                var eventInfo = type.GetEvent(eventBind.Name);
                var methodInfo = _guiHandler.GetType().GetMethod(eventBind.Handler);

                eventInfo.AddEventHandler(control, methodInfo.CreateDelegate(eventInfo.EventHandlerType, _guiHandler));
            }

            if (controlElement.Children.Count > 0)
            {
                if(control is ContentControl contentControl)
                {
                    if (controlElement.Children.Count > 1)
                        throw new InvalidOperationException(string.Format("{0}: Control does not support multiple children.", type.FullName));

                    contentControl.Content = BuildControl(controlElement.Children.First());
                }
                else if(control is ItemsControl itemsControl)
                {
                    foreach (var child in controlElement.Children)
                        itemsControl.Items.Add(BuildControl(child));
                }
                else
                {
                    throw new InvalidOperationException(string.Format("{0}: Control does not support children.", type.FullName));
                }
            }

            return control;
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
            ZOrder = 1;
            return base.OnPointerDown(context, args);
        }

        public void Close()
        {
            this.RemoveFromParent();
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

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            if(_justOpened == false)
            {
                _justOpened = true;
                ZOrder = 1;
                context.SetFocus(this._clientArea.Content as Control);
                Load?.Invoke(this, EventArgs.Empty);
            }

            if(ZOrder == 1 && !_captionBorder.IsDragging)
            {
                if(context.FocusedControl != null)
                {
                    if(context.FocusedControl.IsPartOf<Window>() && !HasFocus)
                    {
                        ZOrder = 0;
                    }
                }
            }
            else
            {
                if(HasFocus || _captionBorder.IsDragging)
                {
                    foreach(var ctrl in this.Parent.Children)
                    {
                        if(ctrl is Window win && win != this)
                        {
                            win.ZOrder = 0;
                        }
                    }
                    ZOrder = 1;
                }
            }

            if(_shown == false)
            {
                _shown = true;
                Load?.Invoke(this, EventArgs.Empty);
            }

            if(_guiHandler != null)
            {
                _guiHandler.Update(deltaSeconds);
            }

            base.Update(context, deltaSeconds);
        }
    }
}
