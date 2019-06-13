using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using System;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Windowing;
using ThePeacenet.Gui.Controls;
using ThePeacenet;
using ThePeacenet.Desktop;
using System.Linq;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend;

namespace ThePeacenet.SystemWindows
{
    public class AppMenuWindow : Window
    {
        private BackgroundBlur _windowBG = null;
        private Border _shroud = null;
        private DockPanel _docker = null;
        private DesktopScreen _desktop = null;
        private ContentManager _content = null;

        public override Control Template => _windowBG;

        protected Overlay StatusBar => FindControl<Overlay>("WhiskerStatusBar");
        protected TextBox Search => FindControl<TextBox>("WhiskerSearch");
        protected StackPanel Sidebar => FindControl<StackPanel>("WhiskerSidebar");
        protected StackPanel Menu => FindControl<StackPanel>("WhiskerMainMenu");

        private string _currentCategory = "All";

        public AppMenuWindow(ContentManager content, DesktopScreen desktop)
        {
            _content = content;
            _desktop = desktop;
            _windowBG = new BackgroundBlur
            {
                Content = _shroud = new Border
                {
                    StyleClass = "AppMenuBG",
                    Content = _docker = new DockPanel
                    {
                        LastChildFill = true
                    }
                }
            };

            _docker.Items.Add(new TextBox
            {
                Name = "WhiskerSearch",
                Text = "Search..."
            });

            _docker.Items.Add(new Overlay
            {
                Name = "WhiskerStatusBar"
            });

            _docker.Items.Add(new StackPanel
            {
                Name = "WhiskerSidebar"
            });

            _docker.Items.Add(new StackPanel
            {
                Name = "WhiskerMainMenu"
            });

            Search.SetAttachedProperty(DockPanel.DockProperty, Dock.Top);
            StatusBar.SetAttachedProperty(DockPanel.DockProperty, Dock.Bottom);
            Sidebar.SetAttachedProperty(DockPanel.DockProperty, Dock.Right);
        }

        public void Repopulate()
        {
            // Clear any existing menu items.
            Menu.Items.Clear();

            // A category that represents *all* items.
            var allCategory = new Button
            {
                Content = new StatusIcon
                {
                    IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/folder"), 16),
                    Content = "All"
                }
            };

            Sidebar.Items.Add(allCategory);

            allCategory.Clicked += (sender, e) => { SetCategory("All"); };

            // Add all of the category items.
            foreach(var category in _desktop.User.Programs.Select(x=>x.LauncherCategory).Distinct())
            {
                var catButton = new Button
                {
                    Content = new StatusIcon
                    {
                        IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/folder"), 16),
                        Content = category
                    }
                };
                catButton.Clicked += (sender, e) => SetCategory(category);
                Sidebar.Items.Add(catButton);
            }

            allCategory.Click();

            Load += (sender, e) => IsFocused = true;

            MinWidth = MaxWidth = 325;
            MinHeight = MaxHeight = 450;
        }

        public void SetCategory(string category)
        {
            _currentCategory = category;

            Menu.Items.Clear();

            foreach(var program in _desktop.User.Programs.Where(x=>x.LauncherCategory == category || category == "All"))
            {
                var appButton = new Button
                {
                    Content = new StatusIcon
                    {
                        IconBrush = new Brush(_content.Load<Texture2D>(program.LauncherIcon), 16),
                        Content = program.Name
                    }
                };

                appButton.Clicked += (sender, e) =>
                {
                    if(_desktop.User.Execute(program.Id, out IProcess process))
                    {
                        process.Run(new NullConsoleContext(_desktop.User), new string[] { program.Id });
                    }
                };

                Menu.Items.Add(appButton);
            }
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            this.Position = new Point(
                    _desktop.DesktopPanelRect.Left,
                    _desktop.DesktopPanelRect.Bottom
                );

            // If the last window in the screen isn't this then we're not in focus.
            if(Screen.Windows.Last() != this || Screen.Content.HasFocus)
            {
                Close();
            }

            base.Update(context, deltaSeconds);
        }
    }
}
