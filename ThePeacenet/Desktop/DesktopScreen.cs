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
        private Shell _shell = null;

        public DesktopScreen(ContentManager content, IUserLand ownerUser)
        {
            _owner = ownerUser;
            
            Content = new Border
            {
                BackgroundBrush = new Brush(Color.White, content.Load<Texture2D>("wallpapers/1"), new Thickness(0), Size2.Empty, BrushType.Image),
                Name = "Wallpaper",
                Content = new StackPanel
                {
                    Orientation = Orientation.Vertical,
                    Name = "RootStacker"
                }
            };

            FindControl<StackPanel>("RootStacker").Items.Add(new Border
            {
                BackgroundBrush = new Brush(new Color(0x22, 0x22, 0x22, 0xff)),
                MinHeight = 24,
                Content = new StackPanel
                {
                    Name = "DesktopPanel",
                    Orientation = Orientation.Horizontal
                }
            });

            FindControl<StackPanel>("RootStacker").Items.Add(new ConsoleControl(content, ownerUser)
            {
                Name = "TestConsole"
            });

            FindControl<StackPanel>("RootStacker").SetFill(FindControl<ConsoleControl>("TestConsole"), 1);

            FindControl<StackPanel>("DesktopPanel").Items.Add(new Button
            {
                Name = "AppButton",
                VerticalAlignment = VerticalAlignment.Centre,
                BackgroundBrush = new Brush(content.Load<Texture2D>("Gui/Textures/menu"))
            });

            FindControl<StackPanel>("DesktopPanel").Items.Add(new StatusIcon
            {
                Content = "Cover",
                Name = "CoverMeter",
                IconBrush = new Brush(content.Load<Texture2D>("Gui/Icons/eye-slash"), 16)
            });

            IProcess shellProcess = null;
            FindControl<ConsoleControl>("TestConsole").Console.Execute("sh", out shellProcess);
            if(shellProcess is Shell shell)
            {
                _shell = shell;
                _shell.Run(FindControl<ConsoleControl>("TestConsole").Console, new[] { "sh" });
            }
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            _shell.Update(deltaSeconds);
        }
    }
}
