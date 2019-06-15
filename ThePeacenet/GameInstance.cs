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
        private static GameInstance _instance = null;

        public static GameInstance Get()
        {
            // Someone's going to fucking murder me for turning the MonoGame game loop into a fucking singleton pattern.
            if (_instance == null) _instance = new GameInstance();
            return _instance;
        }

        private readonly GraphicsDeviceManager _graphics;
        private GuiSystem _guiSystem = null;
        private WorldState _worldState = null;
        private UserContext _playerUserLand = null;
        private IGuiRenderer _renderer = null;
        private DynamicSpriteFont _defaultFont = null;
        private ViewportAdapter _viewportAdapter = null;
        public GraphicsDeviceManager GraphicsManager => _graphics;
        
        private GameInstance()
        {
            _graphics = new GraphicsDeviceManager(this);
            _graphics.HardwareModeSwitch = false;

            IsMouseVisible = true;

            Content.RootDirectory = "Content";
        }

        private void Window_ClientSizeChanged(object sender, EventArgs e)
        {
            ResetViewport();
            _guiSystem.ClientSizeChanged();
        }

        protected override void Initialize()
        {
            Window.AllowUserResizing = true;
            Window.ClientSizeChanged += Window_ClientSizeChanged;

            _defaultFont = Content.LoadFont("DefaultFont");
            
            _renderer = new GuiSpriteBatchRenderer(GraphicsDevice, () =>
            {
                return Matrix.Identity;
            }, Content.Load<Effect>("Effects/Blur"));

            IsMouseVisible = true;
            GameConfig.Apply(this);

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
            ResetViewport();
            _guiSystem.ClientSizeChanged();

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

        protected override void OnExiting(object sender, EventArgs args)
        {
            if(_worldState.IsInGame)
            {
                _worldState.Save();
            }
            base.OnExiting(sender, args);
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
