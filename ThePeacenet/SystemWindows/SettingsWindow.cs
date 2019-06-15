using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui.Controls;
using ThePeacenet.Gui.Windowing;

namespace ThePeacenet.SystemWindows
{
    public sealed class SettingsWindow : Window
    {
        private MainMenu _mainMenu = null;

        public DockPanel Root => Content as DockPanel;

        public StackPanel ScreenResolutionPanel => FindControl<StackPanel>("ScreenResolutionPanel");
        public StackPanel MainPanel => FindControl<StackPanel>("Main");

        public SettingsWindow(MainMenu menu, ContentManager content)
        {
            _mainMenu = menu;

            WindowTitle = "System Settings - Under Construction";
            WindowIcon = content.Load<Texture2D>("Gui/Icons/cogs");

            Content = new DockPanel { LastChildFill = true };

            Root.Items.Add(new StackPanel { Name = "ScreenResolutionPanel" });
            Root.Items.Add(new StackPanel { Name = "Main" });

            ScreenResolutionPanel.SetAttachedProperty(DockPanel.DockProperty, Dock.Left);

            ScreenResolutionPanel.MaxWidth = 130;

            ScreenResolutionPanel.Items.Add(new Label { Content = "Screen Resolution", StyleClass = "Heading2" });
            ScreenResolutionPanel.Items.Add(new Label { Content = "This feature is not yet implemented." });

            MainPanel.Items.Add(new Label { Content = "Graphics", StyleClass = "Heading2" });
            MainPanel.Items.Add(new CheckLabel { Content = "Fullscreen", Checked = true, Name = "IsFullscreen" });
        }
    }
}
