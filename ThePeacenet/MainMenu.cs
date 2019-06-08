using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet
{
    public class MainMenu : Screen
    {
        private readonly ContentManager _content = null;
        private readonly WorldState _world = null;

        public Canvas RootCanvas => FindControl<Canvas>("RootCanvas");
        public StackPanel UsersPanel => FindControl<StackPanel>("UsersPanel");
        public StackPanel ActionsPanel => FindControl<StackPanel>("ActionsPanel");
        public Border PromptBorder => FindControl<Border>("PromptBorder");
        public Image PeacenetLogo => FindControl<Image>("PeacenetLogo");

        public MainMenu(ContentManager content, WorldState world)
        {
            Skin = content.Load<GuiSkin>("Skins/Menu");
            _content = content;
            _world = world;

            Content = new Border
            {
                //StyleClass = "Wallpaper",
                Padding = new Thickness(25),
                Content = new Canvas
                {
                    Name = "RootCanvas"
                }
            };

            RootCanvas.Items.Add(new StackPanel
            {
                Name = "UsersPanel",
                Orientation = Orientation.Vertical
            });

            RootCanvas.Items.Add(new Border
            {
                Name = "PromptBorder"
            });

            RootCanvas.Items.Add(new StackPanel
            {
                Name = "ActionsPanel",
                Orientation = Orientation.Vertical
            });

            RootCanvas.Items.Add(new Image
            {
                Name = "PeacenetLogo",
                BackgroundBrush = new Brush(_content.Load<Texture2D>("Gui/Textures/Peacenet"))
            });

            PeacenetLogo.SetAttachedProperty(Canvas.AnchorProperty, CanvasAnchors.TopCenter);
            PeacenetLogo.SetAttachedProperty(Canvas.AlignmentProperty, CanvasAnchors.TopCenter);
            UsersPanel.SetAttachedProperty(Canvas.AnchorProperty, CanvasAnchors.BottomLeft);
            UsersPanel.SetAttachedProperty(Canvas.AlignmentProperty, CanvasAnchors.BottomLeft);
            ActionsPanel.SetAttachedProperty(Canvas.AnchorProperty, CanvasAnchors.BottomRight);
            ActionsPanel.SetAttachedProperty(Canvas.AlignmentProperty, CanvasAnchors.BottomRight);
            PromptBorder.SetAttachedProperty(Canvas.AnchorProperty, CanvasAnchors.Center);
            PromptBorder.SetAttachedProperty(Canvas.AlignmentProperty, CanvasAnchors.Center);

            UsersPanel.Items.Add(new Button
            {
                Name = "NewIdentity",
                Content = new StatusIcon
                {
                    Content = "New Agent",
                    IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/plus-circle"), 48)
                }
            });

        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }
    }
}
