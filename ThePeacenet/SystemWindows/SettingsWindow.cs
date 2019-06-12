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
    class SettingsWindow : Window
    {
        private MainMenu _mainMenu = null;

        protected StackPanel Root => Content as StackPanel;
    
        protected Button Confirm => FindControl<Button>("Confirm");
        protected Label Error => FindControl<Label>("ErrorText");

        public SettingsWindow(MainMenu menu, ContentManager content)
        {
            _mainMenu = menu;
            WindowTitle = "Settings";
            WindowIcon = content.Load<Texture2D>("Gui/Icons/gears");

            this.MinHeight = 0;

            Content = new StackPanel
            {
                Orientation = Orientation.Vertical,
                Spacing = 5
            };

            Root.Items.Add(new Label
            {
                Content = "We'll get this done once we have a few more essential controls implemented.",
                HorizontalTextAlignment = Gui.HorizontalAlignment.Left
            });

            Root.Items.Add(new Button
            {
                Content = new StatusIcon
                {
                    Content = "Confirm",
                    IconBrush = new Gui.Brush(content.Load<Texture2D>("Gui/Icons/check"), 16)
                },
                HorizontalAlignment = Gui.HorizontalAlignment.Right,
                VerticalAlignment = Gui.VerticalAlignment.Bottom,
                Name = "Confirm"
            });

            Confirm.Clicked += Confirm_Clicked;
        }

        private void Confirm_Clicked(object sender, EventArgs e)
        {
            Close();
        }
    }
}
