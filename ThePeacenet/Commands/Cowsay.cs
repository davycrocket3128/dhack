using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Description("A configurable talking cow.")]
    [UnlockedByDefault]
    [Usage("<text>")]
    public class Cowsay : Command
    {
        protected override void OnRun(string[] args)
        {
            // No sense saving anything that was here, commands work way different from the UE4 codebase.
            // In fact, I ported this thing so well, that I ended up making a one-liner.
            Console.WriteLine(MakeSpeech(String.Join(" ", args), GetCow()));
        }

        protected string GetCow()
        {
            return "\\  ^__^\r\n \\ (oo)\\_______\r\n   (__)\\       )\\/\\\r\n       ||----w |\r\n       ||     ||";
        }

        List<string> SplitLines(string InText, int InWrap)
        {
            List<string> Lines = new List<string> { };
            string Current = "";

            for (int i = 0; i < InText.Length; i++)
            {
                char Char = InText[i];

                switch (Char)
                {
                    case '\0':
                    case '\b':
                    case '\t':
                    case '\v':
                    case '\r':
                        continue;
                    case '\n':
                        Lines.Add(Current);
                        Current = "";
                        continue;
                    default:
                        Current += Char;
                        break;
                }

                if (Current.Length >= InWrap && InWrap > 0)
                {
                    Lines.Add(Current);
                    Current = "";
                }
            }

            if (Current.Length > 0)
            {
                Lines.Add(Current);
                Current = "";
            }

            return Lines;
        }

        String MakeSpeech(string InSpeech, string InCow)
        {
            // Split the cow into individual lines of text. Ouch.
            List<string> cowlines = SplitLines(InCow, 0);

            // In UE4 this was an FString because UE4 doesn't have StringBuilder, but StringBuilder is better for this. - Michael
            StringBuilder sb = new StringBuilder();

            // Wow.  UE4's libraries are so similar to C#'s!
            // But this is C#! Yay MonoGame! Good to know though.
            int length = Math.Min(InSpeech.Length, 30);

            // Add the top of the speech bubble.
            sb.Append(" _");

            for (int i = 0; i < length; i++)
            {
                sb.Append("_");
            }

            sb.AppendLine("_ ");

            // Now
            // we
            // split
            // the
            // speech
            // into
            // lines.
            List<string> lines = SplitLines(InSpeech, length);

            // And go through each line.
            for (int i = 0; i < lines.Count; i++)
            {
                char begin = '|';
                char end = '|';

                if (i == 0)
                {
                    if (lines.Count == 1)
                    {
                        begin = '<';
                        end = '>';
                    }
                    else
                    {
                        begin = '/';
                        end = '\\';
                    }
                }
                else if (i == lines.Count - 1)
                {
                    begin = '\\';
                    end = '/';
                }

                string line = lines[i];

                int lineLength = line.Length;
                int pad = Math.Abs(length - lineLength); // Make damn sure this pad count is positive.

                sb.Append(begin + " " + line);

                for (int j = 0; j < pad; j++)
                {
                    sb.Append(" ");
                }

                sb.AppendLine(" " + end);
                
            }

            sb.Append(" -");
            for (int i = 0; i < length; i++)
            {
                sb.Append("-");
            }
            sb.AppendLine("- ");

            foreach(string cowline in cowlines)
            {
                for (int i = 0; i < length + 4; i++)
                {
                    sb.Append(" ");
                }
                sb.AppendLine(cowline);
            }

            return sb.ToString();
        }
    }
}