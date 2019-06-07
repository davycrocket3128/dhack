using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using MonoGame.Extended.ViewportAdapters;
using ThePeacenet.Backend;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;
using ThePeacenet.Console;
using SpriteFontPlus;
using System.IO;
using System;
using ThePeacenet.Gui;
using ThePeacenet.Desktop;
using ThePeacenet.Backend.AssetTypes;

namespace ThePeacenet
{
    /// <summary>
    /// This is the main type for your game.
    /// </summary>
    public class GameInstance : Game, IProgramGuiBuilder
    {
        private readonly GraphicsDeviceManager _graphics;
        private GuiSystem _guiSystem = null;
        private WorldState _worldState = null;
        private IUserLand _playerUserLand = null;
        private IGuiRenderer _renderer = null;
        private DynamicSpriteFont _defaultFont = null;
        private ViewportAdapter _viewportAdapter = null;
        public GraphicsDeviceManager GraphicsManager => _graphics;
        
        public GameInstance()
        {
            _graphics = new GraphicsDeviceManager(this);
            // No hardware mode switch because borderless fullscreen is better than
            // hardware fullscreen.
            GraphicsManager.HardwareModeSwitch = false;
            
            Content.RootDirectory = "Content";
            IsMouseVisible = true;

            Window.AllowUserResizing = true;
            Window.ClientSizeChanged += Window_ClientSizeChanged;
        }

        private void Window_ClientSizeChanged(object sender, EventArgs e)
        {
            ResetViewport();
            _guiSystem.ClientSizeChanged();
        }

        protected override void Initialize()
        {
            _defaultFont = Content.LoadFont("DefaultFont");
            
            _renderer = new GuiSpriteBatchRenderer(GraphicsDevice, () =>
            {
                return _viewportAdapter.GetScaleMatrix();
            });

            IsMouseVisible = true;
            GameConfig.Apply(this);

            ResetViewport();

            base.Initialize();
        }

        public void ResetViewport()
        {
            System.Console.WriteLine("Updating GUI viewport settings...");

            _viewportAdapter = new DefaultViewportAdapter(
                    GraphicsDevice
                );

            if (_guiSystem != null)
            {
                var defaultFont = _guiSystem.DefaultFont;
                var activeScreen = _guiSystem.ActiveScreen;
                var focusedControl = _guiSystem.FocusedControl;

                _guiSystem = new GuiSystem(_viewportAdapter, _renderer, Content)
                {
                    ActiveScreen = activeScreen
                };
                _guiSystem.SetFocus(focusedControl);
            }
            else
            {
                _guiSystem = new GuiSystem(_viewportAdapter, _renderer, Content);
            }
        }

        protected override void LoadContent()
        {
            _guiSystem.ActiveScreen = new LoadingScreen(Content);

            _worldState = new WorldState(this);

            _worldState.PreloadFinished += (o, a) =>
            {
                this._guiSystem.ActiveScreen = new MainMenu(Content, _worldState);
            };

            _worldState.PlayerSystemReady += (userland) =>
            {
                _playerUserLand = userland;

                _guiSystem.ActiveScreen = new DesktopScreen(Content, _playerUserLand);
            };

            _worldState.Initialize(Content);
        }

        protected override void UnloadContent()
        {
        }

        protected override void Update(GameTime gameTime)
        {
            _worldState.Update(gameTime.GetElapsedSeconds());
            _guiSystem.Update(gameTime);

            

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.Black);

            _guiSystem.Draw(gameTime);
            
            base.Draw(gameTime);
        }

        public IProcess BuildProgram(Backend.AssetTypes.Program program)
        {
            return new WindowProcess(program, this._guiSystem.ActiveScreen as DesktopScreen);
        }
    }
}
