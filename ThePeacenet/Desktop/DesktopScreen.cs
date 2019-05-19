using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;
using ThePeacenet.Console;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Desktop
{
    public class DesktopScreen : Screen
    {
        private IUserLand _owner = null;
        
        public DesktopScreen(ContentManager content, IUserLand ownerUser)
        {
            _owner = ownerUser;
            
            Content = new Border
            {
                BackgroundBrush = new Brush(Color.White, content.Load<Texture2D>("wallpapers/1"), new Thickness(0), Size2.Empty, BrushType.Image),
                Name = "Wallpaper",
                Content = new DockPanel
                {
                    Name = "Root",
                    LastChildFill = true
                }
            };

            FindControl<DockPanel>("Root").Items.Add(new Border
            {
                BackgroundBrush = new Brush(new Color(0x22, 0x22, 0x22, 0xff)),
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
                VerticalAlignment = VerticalAlignment.Centre,
                BackgroundBrush = new Brush(content.Load<Texture2D>("Gui/Textures/menu"))
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

        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }
    }
}
