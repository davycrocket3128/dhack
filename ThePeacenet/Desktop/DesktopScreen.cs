using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;
using ThePeacenet.Console;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;
using ThePeacenet.Gui.Windowing;
using ThePeacenet.SystemWindows;

namespace ThePeacenet.Desktop
{
    public class DesktopScreen : Screen
    {
        private ContentManager _content = null;
        private IUserLand _owner = null;

        protected AppMenuWindow AppMenu { get; }

        public IUserLand User => _owner;
        public Rectangle DesktopPanelRect => FindControl<Border>("DesktopPanelBorder").BoundingRectangle;
        public Button AppButton => FindControl<Button>("AppButton");

        public DesktopScreen(ContentManager content, IUserLand ownerUser)
        {
            _content = content;
            Skin = content.Load<GuiSkin>("Skins/Serenity");
            WindowTheme.Current = new SerenityWindowTheme(content);

            _owner = ownerUser;

            Content = new Border
            {
                StyleClass = "Wallpaper",
                Name = "Wallpaper",
                Content = new DockPanel
                {
                    Name = "Root",
                    LastChildFill = true
                }
            };

            FindControl<DockPanel>("Root").Items.Add(new Border
            {
                MinHeight = 24,
                Name = "DesktopPanelBorder",
                Content = new DockPanel
                {
                    Name = "DesktopPanel",
                    LastChildFill = true
                }
            });

            FindControl<DockPanel>("Root").Items.Add(new Canvas
            {
                Name = "WindowManagerArea",
            });

            FindControl<Border>("DesktopPanelBorder").SetAttachedProperty(DockPanel.DockProperty, Dock.Top);

            FindControl<DockPanel>("DesktopPanel").Items.Add(new Button
            {
                Name = "AppButton",
                StyleClass = "AppButton",
                VerticalAlignment = VerticalAlignment.Centre,
            });

            FindControl<DockPanel>("DesktopPanel").Items.Add(new StackPanel
            {
                Orientation = Orientation.Horizontal,
                Name = "Tray",
                Spacing = 4
            });

            FindControl<StackPanel>("Tray").SetAttachedProperty(DockPanel.DockProperty, Dock.Right);

            FindControl<StackPanel>("Tray").Items.Add(new StatusIcon
            {
                Name = "Username",
                Content = "Username here",
                IconBrush = new Brush(content.Load<Texture2D>("Gui/Icons/user-circle"), 16)
            });

            FindControl<DockPanel>("DesktopPanel").Items.Add(new StackPanel
            {
                Orientation = Orientation.Horizontal,
                Name = "WindowList",
                Spacing = 3,
                VerticalAlignment = VerticalAlignment.Stretch
            });

            AppMenu = new AppMenuWindow(content, this);

            AppButton.Clicked += (sender, e) =>
            {
                if (Windows.Contains(AppMenu))
                {
                    AppMenu.Close();
                }
                else
                {
                    ShowWindow(AppMenu);
                    AppMenu.SetCategory("All");
                }
            };

            ResetAppLauncher();

            AppButton.FocusOnClick = false; // so that the app menu window can get focus.
        }

        public void ResetAppLauncher()
        {
            AppMenu.Repopulate();
        }
    }
}
