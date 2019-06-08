using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using SpriteFontPlus;
using ThePeacenet.Backend;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.OS;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.Console
{
    public sealed class ConsoleControl : Control, IConsoleContext
    {
        private struct TerminalInfo
        {
            public int Lines;
            public float Height;
            public float CharWidth;
            public float CharHeight;

            public TerminalInfo(int lines, float height, float cw, float ch)
            {
                Lines = lines;
                Height = height;
                CharWidth = cw;
                CharHeight = ch;
            }
        }

        private string _work = "/";
        private IUserLand _owner = null;
        private DynamicSpriteFont _regularFont = null;
        private DynamicSpriteFont _boldFont = null;
        private DynamicSpriteFont _italicFont = null;
        private DynamicSpriteFont _boldItalicFont = null;
        private string _textData = "";
        private float _scrollOffsetY = 0;
        private float _zoomFactor = 1;
        private string _textInputBuffer = "";

        public IEnumerable<ThePeacenet.Backend.AssetTypes.Program> Programs => _owner.Programs;

        public string WorkingDirectory
        {
            get => _work;
            set
            {
                if (value != _work && FileSystem.DirectoryExists(value))
                {
                    _work = value;
                }
            }
        }

        private void DrawCharacter(IGuiRenderer renderer, RectangleF rect, DynamicSpriteFont font, Color bg, Color fg, char c)
        {
            renderer.FillRectangle(rect, bg);
            renderer.DrawString(font, c.ToString(), new Vector2(rect.Left, rect.Top), fg, null);
        }

        
        public string Username => _owner.Username;

        public string Hostname => _owner.Hostname;

        public string HomeFolder => _owner.HomeFolder;

        public string IdentityName => _owner.IdentityName;

        public IFileSystem FileSystem => _owner.FileSystem;

        public string EmailAddress => _owner.EmailAddress;

        public bool IsAdmin => _owner.IsAdmin;

        public ConsoleColor UserColor => _owner.UserColor;

        public IEnumerable<CommandAsset> Commands => _owner.Commands;

        public void Clear()
        {
            _textData = "";
        }

        public void Dispose()
        {
        }

        private TerminalInfo GetTerminalInfo(IGuiRenderer renderer)
        {
            TerminalInfo ret = new TerminalInfo(0, 0, 0, 0);

            // Measure a character with the regular font.
            var measureTest = _regularFont.MeasureString("#");

            // The state of the terminal render.
            Vector2 size = new Vector2(BoundingRectangle.Width, BoundingRectangle.Height);
            DynamicSpriteFont font = _regularFont;
            float charX = BoundingRectangle.Left;
            float charY = BoundingRectangle.Top - _scrollOffsetY;
            float charW = measureTest.X * _zoomFactor;
            float charH = measureTest.Y * _zoomFactor;
            Color fgColor = Color.White;
            Color bgColor = Color.Black;

            // Loop through every character in the text buffer.
            for (int i = 0; i < _textData.Length; i++)
            {
                // Get the current character.
                char c = _textData[i];

                // Handle certain whitespace chars.
                switch (c)
                {
                    case '\t':
                        float tabSpace = (charX % (charW * 8));
                        charX += (charW * 8) - tabSpace;
                        continue;
                    case '\r':
                        charX = BoundingRectangle.Left;
                        continue;
                    case '\n':
                        charX = BoundingRectangle.Left;
                        charY += charH;
                        ret.Lines++;
                        ret.Height += charH;
                        continue;
                }

                // Measure the character so we know its width and height.
                var measure = font.MeasureString(c.ToString());
                charW = measure.X * _zoomFactor;
                charH = measure.Y * _zoomFactor;

                // If the vertical position of the cursor is on-screen then we can draw.
                if (charY >= BoundingRectangle.Top && charY <= BoundingRectangle.Bottom && renderer != null)
                {
                    DrawCharacter(renderer, new RectangleF(charX, charY, charW, charH), font, bgColor, fgColor, c);
                }

                // Now we advance the text position.
                if (charX + charW >= BoundingRectangle.Right)
                {
                    ret.Lines++;
                    ret.Height += charH;
                    charY += charH;
                    charX = BoundingRectangle.Left;
                }
                else
                {
                    charX += charW;
                }
            }

            // One final draw of the cursor if it's on-screen.
            if (charY >= BoundingRectangle.Top && charY <= BoundingRectangle.Bottom && renderer != null)
            {
                if (IsFocused)
                {
                    DrawCharacter(renderer, new RectangleF(charX, charY, charW, charH), font, fgColor, bgColor, ' ');
                }
                else
                {
                    renderer.DrawRectangle(new Rectangle((int)charX, (int)charY, (int)charW, (int)charH), fgColor, 1);
                }
            }

            ret.Height += charH;
            ret.Lines++;
            ret.CharWidth = charW;
            ret.CharHeight = charH;

            return ret;
        }

        public bool GetLine(out string text)
        {
            if (_textInputBuffer.Contains("\n"))
            {
                text = _textInputBuffer.Substring(0, _textInputBuffer.IndexOf("\n"));
                _textInputBuffer = _textInputBuffer.Remove(0, _textInputBuffer.IndexOf("\n") + 1);
                return true;
            }
            text = "";
            return false;
        }

        public void OverWrite(string text)
        {
            Write(text + "\r");

        }

        public void OverWrite(string format, params object[] args)
        {
            OverWrite(string.Format(format, args));
        }

        public void Write(string text)
        {
            _textData += text;
            _scrollOffsetY = -1;
        }

        public void Write(string format, params object[] args)
        {
            Write(string.Format(format, args));
        }

        public void WriteLine(string text)
        {
            Write(text + "\r\n");
        }

        public void WriteLine(string format, params object[] args)
        {
            WriteLine(string.Format(format, args));
        }

        public bool Execute(string program, out IProcess process)
        {
            return _owner.Execute(program, out process);
        }

        public ConsoleControl()
        {

        }

        public ConsoleControl(ContentManager content, IUserLand owner) : this()
        {
            Build(content, owner);
        }

        public void Build(ContentManager content, IUserLand owner)
        {
            _owner = owner;

            _regularFont = content.LoadFont("Gui/Fonts/Terminal/Regular");
            _boldFont = content.LoadFont("Gui/Fonts/Terminal/Bold");
            _boldItalicFont = content.LoadFont("Gui/Fonts/Terminal/BoldItalic");
            _italicFont = content.LoadFont("Gui/Fonts/Terminal/Italic");

            var measure = _regularFont.MeasureString("#");
            MinWidth = (int)(measure.X * 80);
            MinHeight = (int)(measure.Y * 25);
        }

        public override IEnumerable<Control> Children => Enumerable.Empty<Control>();

        public override Size2 GetContentSize(IGuiContext context)
        {
            return new Size2();
        }

        public override void OnScrolled(int delta)
        {
            _scrollOffsetY -= delta;
            if (_scrollOffsetY < 0) _scrollOffsetY = 0;
            base.OnScrolled(delta);
        }

        public override bool OnKeyPressed(IGuiContext context, KeyboardEventArgs e)
        {
            if (e.Modifiers.HasFlag(KeyboardModifiers.Control))
            {
                if (e.Key == Microsoft.Xna.Framework.Input.Keys.OemPlus)
                {
                    _zoomFactor += 0.25f;
                    _regularFont.Size = _regularFont.Size;
                    _boldFont.Size = _regularFont.Size;
                    _boldItalicFont.Size = _regularFont.Size;
                    _italicFont.Size = _regularFont.Size;

                    var measure = _regularFont.MeasureString("#");
                    MinWidth = (int)(measure.X * 80);
                    MinHeight = (int)(measure.Y * 25);
                    return true;
                }
                else if (e.Key == Microsoft.Xna.Framework.Input.Keys.OemMinus)
                {
                    if (_zoomFactor - 0.25f >= 1) _zoomFactor -= 0.25f;
                    var measure = _regularFont.MeasureString("#");
                    MinWidth = (int)(measure.X * 80);
                    MinHeight = (int)(measure.Y * 25);
                    return true;
                }
                _scrollOffsetY = -1;
            }
            return base.OnKeyPressed(context, e);
        }

        public override bool OnKeyTyped(IGuiContext context, KeyboardEventArgs e)
        {
            if (e.Character == null) return false;

            char c = (char)e.Character;

            if (c == '\b' && !string.IsNullOrEmpty(_textInputBuffer))
            {
                _textInputBuffer = _textInputBuffer.Remove(_textInputBuffer.Length - 1);
                _textData = _textData.Remove(_textData.Length - 1);
            }
            else if (c == '\r')
            {
                _textInputBuffer += "\n";
                _textData += "\n";
            }
            else
            {
                _textInputBuffer += c;
                _textData += c;
            }

            _scrollOffsetY = -1;
            return base.OnKeyTyped(context, e);
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
            base.Draw(context, renderer, deltaSeconds);

            // Clear the console.
            renderer.FillRectangle(BoundingRectangle, Color.Black);

            // Draw the terminal.
            GetTerminalInfo(renderer);
        }

        public override void Update(IGuiContext context, float deltaSeconds)
        {
            var info = GetTerminalInfo(null);

            if (_scrollOffsetY < 0)
            {
                _scrollOffsetY = info.Height - BoundingRectangle.Height;
            }

            _scrollOffsetY = MathHelper.Clamp(_scrollOffsetY, 0, info.Height - BoundingRectangle.Height);
            base.Update(context, deltaSeconds);
        }
    }
}
