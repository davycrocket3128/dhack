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
    // No need for a usage string since, unlike Peace Engine, they're not required for
    // commands like this that only use command-line arguments as a source of text.
    public class Cowsay : Command
    {
        protected override void OnRun(string[] args)
        {
            // Cowsay can get speech text from two places.
            // - Command-line arguments, just like echo
            // - Unix pipes.
            // Command-line arguments will take precedence, so if they're empty
            // then we'll attempt to read from the pipe.  If we get nothing then
            // we throw a usage error.

            string speech = string.Empty;
            if(args.Length > 0)
            {
                speech = string.Join(" ", args);
            }
            else
            {
                bool receivedLine = false;
                while(Console.GetLine(out string line))
                {
                    receivedLine = true;
                    speech += line + Environment.NewLine;
                }
                speech = speech.Trim();

                if(!receivedLine)
                {
                    Console.WriteLine(@"cowsay:
    Usage: 
        cowsay <text>
     -- OR --
        command | cowsay");
                    return;
                }
            }

            Console.WriteLine(MakeSpeech(speech, GetCow()));
        }

        protected string GetCow()
        {
            return "\\  ^__^\r\n \\ (oo)\\_______\r\n   (__)\\       )\\/\\\r\n       ||----w |\r\n       ||     ||";
        }

        private List<string> SplitLines(string InText, int InWrap)
        {
            var list = new List<string>();

            var lines = InText.Replace("\r", "").Split('\n');

            foreach(var line in lines)
            {
                if (InWrap > 0)
                {
                    var wrappedLine = "";
                    var words = line.Split(' ');

                    foreach (var word in words)
                    {
                        if (wrappedLine.Length + word.Length + 1 > InWrap) // additional + 1 accounts for the space after the word.
                        {
                            list.Add(wrappedLine);
                            wrappedLine = "";
                        }
                        wrappedLine += word + " ";
                    }
                    list.Add(wrappedLine.Trim());
                }
                else
                {
                    list.Add(line);
                }
            }

            return list;
        }

        private string MakeSpeech(string InSpeech, string InCow)
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