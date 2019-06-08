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

namespace ThePeacenet.Desktop
{
    public class DesktopScreen : Screen
    {
        private ContentManager _content = null;
        private IUserLand _owner = null;
        
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

            FindControl<Canvas>("WindowManagerArea").Items.Add(CreateWhiskerMenu());

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

            ResetAppLauncher();
        }

        public void ResetAppLauncher()
        {
            var categories = FindControl<StackPanel>("Whisker_Categories");
            var items = FindControl<StackPanel>("Whisker_Items");

            categories.Items.Clear();
            items.Items.Clear();

            categories.Items.Add(new Button
            {
                Content = new StatusIcon
                {
                    Content = "All",
                    IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/folder"), 16)
                },
                HorizontalTextAlignment = HorizontalAlignment.Left
            });

            foreach(var cat in _owner.Programs.Select(x => x.LauncherCategory).Distinct())
            {
                categories.Items.Add(new Button
                {
                    Content = new StatusIcon
                    {
                        Content = cat,
                        IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/folder"), 16)
                    },
                    HorizontalTextAlignment = HorizontalAlignment.Left
                });
            }

            foreach(var program in _owner.Programs)
            {
                var button = new Button
                {
                    Content = new StatusIcon
                    {
                        Content = program.Name,
                        IconBrush = new Brush(program.Content.Load<Texture2D>(program.LauncherIcon), 16)
                    }
                };

                button.Clicked += (o, a) =>
                {
                    _owner.Execute(program.Id, out IProcess p);
                    p.Run(new NullConsoleContext(_owner), new[] { program.Id });
                };

                items.Items.Add(button);
            }
        }

        public Control CreateWhiskerMenu()
        {
            var dickPanel = new DockPanel
            {
                LastChildFill = true
            };

            var searchText = new Label
            {
                Name = "WhiskerSearch",
                Content = "Search...",
                HorizontalTextAlignment = HorizontalAlignment.Left,
                Padding = new ThePeacenet.Gui.Thickness(4)
            };

            searchText.SetAttachedProperty(DockPanel.DockProperty, Dock.Top);

            dickPanel.Items.Add(searchText);

            var categories = new StackPanel
            {
                Name = "Whisker_Categories",
                Orientation = Orientation.Vertical,
                Spacing = 3,
                MinWidth = 125,
            };
            categories.SetAttachedProperty(DockPanel.DockProperty, Dock.Right);
            dickPanel.Items.Add(categories);

            var items = new StackPanel
            {
                Name = "Whisker_Items",
                Orientation = Orientation.Vertical,
                Spacing = 3,
            };
            dickPanel.Items.Add(items);

            return new Border
            {
                Content = dickPanel,
                MinHeight = 450,
                MinWidth = 500
            };
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }

        public void ShowWindow(Window win)
        {
            win.SetAttachedProperty(Canvas.AnchorProperty, new Vector2(0.5f, 0.5f));
            win.SetAttachedProperty(Canvas.AlignmentProperty, new Vector2(0.5f, 0.5f));

            FindControl<Canvas>("WindowManagerArea").Items.Add(win);
            FindControl<Canvas>("WindowManagerArea").InvalidateMeasure();
        }
    }
}
