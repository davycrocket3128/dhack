using Microsoft.Xna.Framework;
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
using ThePeacenet.SystemWindows;

namespace ThePeacenet
{
    public class MainMenu : Screen
    {
        private readonly ContentManager _content = null;
        private readonly WorldState _world = null;

        private NewIdentityWindow _newIdentityWindow = null;
        private SaveInfo _activeSave = null;

        public Canvas RootCanvas => FindControl<Canvas>("RootCanvas");
        public StackPanel UsersPanel => FindControl<StackPanel>("UsersPanel");
        public StackPanel ActionsPanel => FindControl<StackPanel>("ActionsPanel");
        public StackPanel ActiveSavePanel => FindControl<StackPanel>("ActiveSave");
        public Image PeacenetLogo => FindControl<Image>("PeacenetLogo");
        public Button NewIdentity => FindControl<Button>("NewIdentity");
        public Label Username => FindControl<Label>("ActiveUsername");
        public Label LastPlayed => FindControl<Label>("ActiveLastPlayed");
        public Button LoadGame => FindControl<Button>("LoadGame");

        public MainMenu(ContentManager content, WorldState world)
        {
            Skin = content.Load<GuiSkin>("Skins/Menu");
            _content = content;
            _world = world;

            Content = new Border
            {
                StyleClass = "Wallpaper",
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

            RootCanvas.Items.Add(new StackPanel
            {
                Name = "ActiveSave",
                Orientation = Orientation.Vertical,
                Spacing = 10
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
            ActiveSavePanel.SetAttachedProperty(Canvas.AnchorProperty, CanvasAnchors.Center);
            ActiveSavePanel.SetAttachedProperty(Canvas.AlignmentProperty, CanvasAnchors.Center);

            foreach(var save in _world.AvailableAgents.OrderByDescending(x=>x.LastPlayed).Take(5).Reverse())
            {
                var btn = new Button
                {
                    Content = new StatusIcon
                    {
                        Content = save.Name,
                        IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/user-circle"), 32)
                    }
                };

                btn.Clicked += (o, a) =>
                {
                    this._activeSave = save;
                };

                UsersPanel.Items.Add(btn);
            }

            UsersPanel.Items.Add(new Button
            {
                Name = "NewIdentity",
                Content = new StatusIcon
                {
                    Content = "New Agent",
                    IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/plus-circle"), 32)
                }
            });

            NewIdentity.Clicked += NewIdentity_Clicked;

            _newIdentityWindow = new NewIdentityWindow(this, content);

            ActiveSavePanel.Items.Add(new Image
            {
                BackgroundBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/user-circle"), 96),
                HorizontalAlignment = HorizontalAlignment.Centre
            });

            ActiveSavePanel.Items.Add(new Label
            {
                Name = "ActiveUsername",
                Content = "Username",
                StyleClass = "Title"
            });

            ActiveSavePanel.Items.Add(new Label
            {
                Name = "ActiveLastPlayed",
                Content = "Last played date",
                StyleClass = "Subtitle"
            });

            ActiveSavePanel.Items.Add(new Button
            {
                Name = "LoadGame",
                Content = new StatusIcon
                {
                    Content = "Login",
                    IconBrush = new Brush(_content.Load<Texture2D>("Gui/Icons/sign-in"), 16)
                }
            });

            LoadGame.Clicked += (o, a) =>
            {
                _world.StartGame(_activeSave);
            };
        }

        private void NewIdentity_Clicked(object sender, EventArgs e)
        {
            ShowWindow(_newIdentityWindow);
        }

        public override void Update(GameTime gameTime)
        {
            base.Update(gameTime);

            ActiveSavePanel.IsVisible = _activeSave != null;

            if(ActiveSavePanel.IsVisible)
            {
                Username.Content = _activeSave.Name;
                LastPlayed.Content = _activeSave.LastPlayed.ToShortDateString() + " - " + _activeSave.LastPlayed.ToShortTimeString();
            }
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }

        public void NewGame(string playerName)
        {
            _world.StartNewGame(playerName);
        }
    }
}
