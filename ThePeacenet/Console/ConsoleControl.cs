using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
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
        private const int MAX_ZOOMLEVEL = 10;

        public const int TERMINAL_DEFAULT_ROWS = 24;
        public const int TERMINAL_DEFAULT_COLUMNS = 80;

        private static List<SpriteFont> _regularZoomLevels = new List<SpriteFont>();
        private static List<SpriteFont> _boldItalicZoomLevels = new List<SpriteFont>();
        private static List<SpriteFont> _boldZoomLevels = new List<SpriteFont>();
        private static List<SpriteFont> _italicZoomLevels = new List<SpriteFont>();

        public static void LoadTerminalFonts(GraphicsDevice graphics, ContentManager content)
        {
            System.Console.WriteLine("Pre-loading console fonts... This is going to take a bit, but it'll dramatically improve in-game performance!");
            var regular = content.Load<byte[]>("Gui/Fonts/Terminal/Regular");
            var bold = content.Load<byte[]>("Gui/Fonts/Terminal/Bold");
            var boldItalic = content.Load<byte[]>("Gui/Fonts/Terminal/BoldItalic");
            var italic = content.Load<byte[]>("Gui/Fonts/Terminal/Italic");

            for(int i = 0; i <= MAX_ZOOMLEVEL; i++)
            {
                float fontSize = 16 + (2 * i);
                int texSize = (int)(1024 + (128 * i));

                System.Console.WriteLine("Baking fonts for zoom level {0} ({1}px size, {2}x{2} texture)...", i, fontSize, texSize);
                var regularBake = TtfFontBaker.Bake(regular, fontSize, texSize, texSize, new[] { CharacterRange.BasicLatin, CharacterRange.Latin1Supplement, CharacterRange.LatinExtendedA, CharacterRange.LatinExtendedB });
                var boldBake = TtfFontBaker.Bake(bold, fontSize, texSize, texSize, new[] { CharacterRange.BasicLatin, CharacterRange.Latin1Supplement, CharacterRange.LatinExtendedA, CharacterRange.LatinExtendedB });
                var italicBake = TtfFontBaker.Bake(italic, fontSize, texSize, texSize, new[] { CharacterRange.BasicLatin, CharacterRange.Latin1Supplement, CharacterRange.LatinExtendedA, CharacterRange.LatinExtendedB });
                var boldItalicBake = TtfFontBaker.Bake(boldItalic, fontSize, texSize, texSize, new[] { CharacterRange.BasicLatin, CharacterRange.Latin1Supplement, CharacterRange.LatinExtendedA, CharacterRange.LatinExtendedB });

                _regularZoomLevels.Add(regularBake.CreateSpriteFont(graphics));
                _boldZoomLevels.Add(boldBake.CreateSpriteFont(graphics));
                _boldItalicZoomLevels.Add(boldItalicBake.CreateSpriteFont(graphics));
                _italicZoomLevels.Add(italicBake.CreateSpriteFont(graphics));
            }
            System.Console.WriteLine("Done preloading fonts!");
        }

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
        private UserContext _owner = null;
        private string _textData = "";
        private float _scrollOffsetY = 0;
        private int _zoomLevel = 0;
        private string _textInputBuffer = "";

        public UserContext User => _owner;

        public string WorkingDirectory
        {
            get => _work;
            set
            {
                if (value != _work && User.FileSystem.DirectoryExists(value))
                {
                    _work = value;
                }
            }
        }

        private void DrawCharacter(IGuiRenderer renderer, RectangleF rect, SpriteFont font, Color bg, Color fg, char c)
        {
            renderer.DrawString(font, c.ToString(), new Vector2(rect.Left, rect.Top), fg);
        }

        public void Clear()
        {
            _textData = "";
        }

        public void Dispose()
        {
        }

        private SpriteFont GetFont(bool bold, bool italic)
        {
            if (_regularZoomLevels.Count == 0) throw new InvalidOperationException("The game hasn't loaded the console fonts yet.");
            if (_boldZoomLevels.Count == 0) throw new InvalidOperationException("The game hasn't loaded the console fonts yet.");
            if (_italicZoomLevels.Count == 0) throw new InvalidOperationException("The game hasn't loaded the console fonts yet.");
            if (_boldItalicZoomLevels.Count == 0) throw new InvalidOperationException("The game hasn't loaded the console fonts yet.");

            var repo = _regularZoomLevels;
            if (bold && italic)
                repo = _boldItalicZoomLevels;
            else if (bold)
                repo = _boldZoomLevels;
            else if (italic)
                repo = _italicZoomLevels;

            return repo[_zoomLevel];
        }

        private TerminalInfo GetTerminalInfo(IGuiRenderer renderer)
        {
            TerminalInfo ret = new TerminalInfo(0, 0, 0, 0);

            // Get the font for the current zoom level.
            var font = GetFont(false, false);

            // Measure a character with the regular font.
            var measureTest = font.MeasureString("#");

            // The state of the terminal render.
            Vector2 size = new Vector2(BoundingRectangle.Width, BoundingRectangle.Height);
            float charX = BoundingRectangle.Left;
            float charY = BoundingRectangle.Top - _scrollOffsetY;
            float charW = measureTest.X;
            float charH = measureTest.Y;
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
                charW = measure.X;
                charH = measure.Y;

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
                    renderer.FillRectangle(new Rectangle((int)charX, (int)charY, (int)charW, (int)charH), fgColor);
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

        public ConsoleControl(ContentManager content, UserContext owner) : this()
        {
            Build(content, owner);
        }

        public void Build(ContentManager content, UserContext owner)
        {
            _owner = owner;

            var measure = GetFont(false, false).MeasureString("#");
            MinWidth = (int)(measure.X * TERMINAL_DEFAULT_COLUMNS);
            MinHeight = (int)(measure.Y * TERMINAL_DEFAULT_ROWS);
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
            return base.OnKeyPressed(context, e);
        }

        public override bool OnKeyTyped(IGuiContext context, KeyboardEventArgs e)
        {
            if (e.Character == null) return false;

            if (e.Modifiers.HasFlag(KeyboardModifiers.Control))
            {
                if (e.Key == Microsoft.Xna.Framework.Input.Keys.OemPlus && _zoomLevel < MAX_ZOOMLEVEL)
                {
                    _zoomLevel++;

                    var measure = GetFont(false, false).MeasureString("#");
                    MinWidth = (int)(measure.X * TERMINAL_DEFAULT_COLUMNS);
                    MinHeight = (int)(measure.Y * TERMINAL_DEFAULT_ROWS);
                    return true;
                }
                else if (e.Key == Microsoft.Xna.Framework.Input.Keys.OemMinus && _zoomLevel > 0)
                {
                    _zoomLevel--;
                    var measure = GetFont(false, false).MeasureString("#");
                    MinWidth = (int)(measure.X * TERMINAL_DEFAULT_COLUMNS);
                    MinHeight = (int)(measure.Y * TERMINAL_DEFAULT_ROWS);
                    return true;
                }
                _scrollOffsetY = -1;
            }

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
            // Draw the background and border.
            base.Draw(context, renderer, deltaSeconds);

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
