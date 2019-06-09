using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using MonoGame.Extended.ViewportAdapters;
using SpriteFontPlus;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Gui
{
    public class GuiSystem : IRectangular, IGuiContext
    {
        private readonly MouseListener _mouseListener;
        private readonly TouchListener _touchListener;
        private readonly KeyboardListener _keyboardListener;
        private ViewportAdapter _viewportAdapter = null;
        private IGuiRenderer _renderer = null;
        private Screen _screen = null;
#if DEBUG
        private bool _debugMode = true;
#else
        private bool _debugMode = false;
#endif

        private Control _preFocusedControl = null;

        public Rectangle BoundingRectangle => _viewportAdapter.BoundingRectangle;

        public ContentManager Content { get; }

        public GuiSystem(ViewportAdapter viewportAdapter, IGuiRenderer renderer, ContentManager content)
        {
            Content = content;
            _viewportAdapter = viewportAdapter;
            _renderer = renderer;

            _mouseListener = new MouseListener(viewportAdapter);
            _mouseListener.MouseDown += (s, e) => OnPointerDown(PointerEventArgs.FromMouseArgs(e));
            _mouseListener.MouseMoved += (s, e) => OnPointerMoved(PointerEventArgs.FromMouseArgs(e));
            _mouseListener.MouseUp += (s, e) => OnPointerUp(PointerEventArgs.FromMouseArgs(e));
            _mouseListener.MouseWheelMoved += (s, e) => FocusedControl?.OnScrolled(e.ScrollWheelDelta);

            _touchListener = new TouchListener(viewportAdapter);
            _touchListener.TouchStarted += (s, e) => OnPointerDown(PointerEventArgs.FromTouchArgs(e));
            _touchListener.TouchMoved += (s, e) => OnPointerMoved(PointerEventArgs.FromTouchArgs(e));
            _touchListener.TouchEnded += (s, e) => OnPointerUp(PointerEventArgs.FromTouchArgs(e));

            _keyboardListener = new KeyboardListener();
            _keyboardListener.KeyPressed += (sender, args) =>
            {
                if(args.Key == Microsoft.Xna.Framework.Input.Keys.F3)
                {
                    _debugMode = !_debugMode;
                    return;
                }
                PropagateDown(FocusedControl, x => x.OnKeyPressed(this, args));
            };
            _keyboardListener.KeyTyped += (sender, args) => PropagateDown(FocusedControl, x => x.OnKeyTyped(this, args));

            DefaultFont = Content.LoadFont("DefaultFont");
        }

        public Point CursorPosition { get; set; }
        public Control FocusedControl { get; private set; }
        public Control HoveredControl { get; private set; }
        public DynamicSpriteFont DefaultFont { get; private set; }

        public Screen ActiveScreen
        {
            get => _screen;
            set
            {
                if(_screen != value)
                {
                    _screen = value;

                    if(_screen != null)
                    {
                        InitializeScreen(_screen);
                    }
                }
            }
        }

        public void InitializeScreen(Screen screen)
        {
            screen.Layout(this, BoundingRectangle);
        }

        public void Update(GameTime gameTime)
        {
            if (_screen == null) return;

            _touchListener.Update(gameTime);
            _mouseListener.Update(gameTime);
            _keyboardListener.Update(gameTime);

            var deltaSeconds = gameTime.GetElapsedSeconds();

            if (ActiveScreen != null && ActiveScreen.IsVisible)
            {
                if(ActiveScreen.IsLayoutRequired)
                {
                    ActiveScreen.Layout(this, BoundingRectangle);
                }

                UpdateControl(ActiveScreen.Content, deltaSeconds);

                foreach(var window in ActiveScreen.Windows)
                {
                    UpdateControl(window, deltaSeconds);
                }
            }

            ActiveScreen.Update(gameTime);
        }

        public void Draw(GameTime gameTime)
        {
            if (_screen == null) return;

            var deltaSeconds = gameTime.GetElapsedSeconds();


            if (ActiveScreen != null && ActiveScreen.IsVisible)
            {
                DrawControl(ActiveScreen.Content, deltaSeconds);

                foreach(var window in ActiveScreen.Windows)
                {
                    DrawControl(window, deltaSeconds);
                }

                _renderer.Begin();

                ActiveScreen.Draw(this, _renderer, deltaSeconds);

                if (_debugMode)
                {
                    string debugText = $@"Project: Greenlight
FPS: {(int)(1 / gameTime.GetElapsedSeconds())}";

                    var measure = DefaultFont.MeasureString(debugText);

                    _renderer.FillRectangle(new RectangleF(0, 0, measure.X, measure.Y), Color.Black * 0.5f);
                    _renderer.DrawString(DefaultFont, debugText, Vector2.Zero, Color.White, null);
                }

                _renderer.End();
            }

        }

        public void UpdateControl(Control control, float deltaSeconds)
        {
            if (control.IsVisible)
            {
                control.ApplySkin(Content, ActiveScreen.Skin);

                control.Update(this, deltaSeconds);

                foreach (var childControl in control.Children.ToArray())
                    UpdateControl(childControl, deltaSeconds);
            }
        }

        public void ClientSizeChanged()
        {
            ActiveScreen?.Layout(this, BoundingRectangle);
        }

        private void DrawControl(Control control, float deltaSeconds)
        {
            if (control.IsVisible)
            {
                _renderer.SetScissorRect(control.BoundingRectangle);

                _renderer.Begin();

                control.Draw(this, _renderer, deltaSeconds);

                if(_debugMode)
                {
                    _renderer.DrawBrush(control.BoundingRectangle, new Brush(Color.White, null, new Thickness(2), Size2.Empty, BrushType.Border));
                    _renderer.DrawBrush(control.ContentRectangle, new Brush(Color.Green, null, new Thickness(1), Size2.Empty, BrushType.Border));
                }

                _renderer.End();

                foreach (var childControl in control.Children.OrderBy(x=>x.ZOrder))
                    DrawControl(childControl, deltaSeconds);

                _renderer.SetScissorRect(BoundingRectangle);
            }
        }

        private void OnPointerDown(PointerEventArgs args)
        {
            if (ActiveScreen == null || !ActiveScreen.IsVisible)
                return;

            _preFocusedControl = FindControlAtPoint(args.Position, true);
            SetFocus(_preFocusedControl);
            PropagateDown(HoveredControl, x => x.OnPointerDown(this, args));
        }

        private void OnPointerUp(PointerEventArgs args)
        {
            if (ActiveScreen == null || !ActiveScreen.IsVisible)
                return;

            var postFocusedControl = FindControlAtPoint(args.Position);

            if (_preFocusedControl == postFocusedControl)
            {
                SetFocus(postFocusedControl);
            }

            _preFocusedControl = null;
            PropagateDown(HoveredControl, x => x.OnPointerUp(this, args));
        }

        private void OnPointerMoved(PointerEventArgs args)
        {
            CursorPosition = args.Position;

            if (ActiveScreen == null || !ActiveScreen.IsVisible)
                return;

            var hoveredControl = FindControlAtPoint(args.Position);

            if (HoveredControl != hoveredControl)
            {
                if (HoveredControl != null && (hoveredControl == null || !hoveredControl.HasParent(HoveredControl)))
                    PropagateDown(HoveredControl, x => x.OnPointerLeave(this, args));

                HoveredControl = hoveredControl;
                PropagateDown(HoveredControl, x => x.OnPointerEnter(this, args));
            }
            else
            {
                PropagateDown(HoveredControl, x => x.OnPointerMove(this, args));
            }
        }

        public void SetFocus(Control focusedControl)
        {
            if (FocusedControl != focusedControl)
            {
                if (FocusedControl != null)
                {
                    FocusedControl.IsFocused = false;
                    PropagateDown(FocusedControl, x => x.OnUnfocus(this));
                }

                FocusedControl = focusedControl;

                if (FocusedControl != null)
                {
                    FocusedControl.IsFocused = true;
                    PropagateDown(FocusedControl, x => x.OnFocus(this));
                }
            }
        }

        /// <summary>
        /// Method is meant to loop down the parents control to find a suitable event control.  If the predicate returns false
        /// it will continue down the control tree.
        /// </summary>
        /// <param name="control">The control we want to check against</param>
        /// <param name="predicate">A function to check if the propagation should resume, if returns false it will continue down the tree.</param>
        private static void PropagateDown(Control control, Func<Control, bool> predicate)
        {
            while (control != null && predicate(control))
            {
                control = control.Parent;
            }
        }

        private Control FindControlAtPoint(Point point, bool shiftWindows = false)
        {
            if (ActiveScreen == null || !ActiveScreen.IsVisible)
                return null;

            foreach(var window in ActiveScreen.Windows.Reverse())
            {
                var found = FindControlAtPoint(window, point);
                if (found != null)
                {
                    if(shiftWindows) ActiveScreen.BringToFront(window);
                    return found;
                }
            }

            return FindControlAtPoint(ActiveScreen.Content, point);
        }

        private Control FindControlAtPoint(Control control, Point point)
        {
            foreach (var controlChild in control.Children.OrderBy(x=>x.ZOrder).Reverse())
            {
                var c = FindControlAtPoint(controlChild, point);

                if (c != null)
                    return c;
            }


            if (control.IsVisible && control.Contains(this, point))
                return control;

            return null;
        }
    }
}
