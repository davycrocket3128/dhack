using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;
using ThePeacenet.Gui.Windowing;

namespace ThePeacenet
{
    public class Infobox : Window
    {
        private Window _ownerWindow = null;

        private DockPanel Root => Content as DockPanel;
        private StackPanel ButtonStacker => FindControl<StackPanel>("ButtonStacker");
        private Image BigIcon => FindControl<Image>("BigIcon");
        private StackPanel ContentStacker => FindControl<StackPanel>("ContentStacker");
        private Label MessageLabel => FindControl<Label>("MessageLabel");

        private Infobox()
        {
            var content = GameInstance.Get().Content;
            WindowTitle = "Information";
            WindowIcon = content.Load<Texture2D>("Gui/Icons/info-circle");

            Content = new DockPanel();

            Root.Items.Add(new StackPanel
            {
                Name = "ButtonStacker",
                Spacing = 3,
                Padding = new Thickness(15),
                HorizontalAlignment = HorizontalAlignment.Centre,
                Orientation = Orientation.Horizontal
            });
            Root.Items.Add(new Image
            {
                Name = "BigIcon",
                Padding = new Thickness(15),
                BackgroundBrush = new Brush(content.Load<Texture2D>("Gui/Icons/warning"), 64),
                MinWidth = 64,
                VerticalAlignment = VerticalAlignment.Centre
            });
            Root.Items.Add(new StackPanel
            {
                Name = "ContentStacker",
                Spacing = 5,
                Padding = new Thickness(15),
                VerticalAlignment = VerticalAlignment.Centre
            });

            BigIcon.SetAttachedProperty(DockPanel.DockProperty, Dock.Left);
            ButtonStacker.SetAttachedProperty(DockPanel.DockProperty, Dock.Bottom);

            ContentStacker.Items.Add(new Label
            {
                Name = "MessageLabel",
                Content = "This is the message of the infobox.",
                HorizontalTextAlignment = HorizontalAlignment.Left
            });
        }

        public string MessageText
        {
            get => MessageLabel.Content as string;
            set => MessageLabel.Content = value;
        }

        private Infobox(Screen owner) : this()
        {
            owner.ShowWindow(this);
        }

        public void AddButton(string icon, string text, Action onClick = null)
        {
            var content = GameInstance.Get().Content;

            var button = new Button
            {
                Content = new StatusIcon
                {
                    IconBrush = new Brush(content.Load<Texture2D>(icon), 16),
                    Content = text
                }
            };

            button.Clicked += (o, a) =>
            {
                onClick?.Invoke();
                Close();
            };

            ButtonStacker.Items.Add(button);
        }

        private Infobox(Window owner) : this(owner.Screen)
        {
            owner.IsEnabled = false;
            _ownerWindow = owner;
        }

        public static Infobox Open(Screen owner)
        {
            return new Infobox(owner);
        }

        public static Infobox Open(Window owner)
        {
            return new Infobox(owner);
        }
    }
}
