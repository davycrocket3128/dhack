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
    public class NewIdentityWindow : Window
    {
        private MainMenu _mainMenu = null;

        protected StackPanel Root => Content as StackPanel;

        protected TextBox PlayerName => FindControl<TextBox>("IdentityName");
        protected Button Confirm => FindControl<Button>("Confirm");
        protected Label Error => FindControl<Label>("ErrorText");

        public NewIdentityWindow(MainMenu menu, ContentManager content)
        {
            _mainMenu = menu;
            WindowTitle = "New Identity";
            WindowIcon = content.Load<Texture2D>("Gui/Icons/user-plus");

            this.MinHeight = 0;

            Content = new StackPanel
            {
                Orientation = Orientation.Vertical,
                Spacing = 5
            };

            Root.Items.Add(new Label
            {
                Content = "Enter Identity Name:",
                HorizontalTextAlignment = Gui.HorizontalAlignment.Left
            });

            Root.Items.Add(new TextBox
            {
                Text = "Player",
                Name = "IdentityName"
            });

            Root.Items.Add(new Label
            {
                Content = "Your name cannot be blank.",
                HorizontalTextAlignment = Gui.HorizontalAlignment.Left,
                StyleClass = "Error",
                Name = "ErrorText",
                IsVisible = false
            });
                
            Root.Items.Add(new Button
            {
                Content = new StatusIcon
                {
                    Content = "Confirm",
                    IconBrush = new Gui.Brush(content.Load<Texture2D>("Gui/Icons/check"), 16)
                },
                HorizontalAlignment = Gui.HorizontalAlignment.Right,
                Name = "Confirm"
            });

            PlayerName.TextChanged += PlayerName_TextChanged;
            Confirm.Clicked += Confirm_Clicked;
        }

        private void PlayerName_TextChanged(object sender, EventArgs e)
        {
            Error.IsVisible = string.IsNullOrWhiteSpace(PlayerName.Text);
            Confirm.IsEnabled = !Error.IsVisible;
        }

        private void Confirm_Clicked(object sender, EventArgs e)
        {
            _mainMenu.NewGame(PlayerName.Text);
            Close();
        }


    }
}
