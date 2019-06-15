using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Shell
{
    public abstract class Shell : Command
    {
        public class ShellParseException : Exception
        {
            public ShellParseException(string message) : base(message)
            {

            }
        }

        struct ShellInstruction
        {
            public bool Overwrites;
            public string OutputFile;
            public string[] Commands;

            public ShellInstruction(string[] commands, string path, bool overwrite)
            {
                Overwrites = overwrite;
                Commands = commands;
                OutputFile = path;
            }

            public static ShellInstruction Empty => new ShellInstruction(new string[] { }, "", false);
        }

        private bool _waitingForInput = false;
        private IConsoleContext _lastConsoleContext = null;
        private List<CommandInstruction> _instructions = new List<CommandInstruction>();
        private bool _waitingForCommand = false;
        private Command _currentCommand = null;
        private IConsoleContext _currentConsole = null;

        protected override bool IsLatent => true;

        protected virtual bool AllowRedirection { get => false; }
        protected virtual bool AllowPipes { get => false; }
        protected virtual bool AutoCompleteSpecialCommands { get => true; }
        protected virtual string ShellPrompt => "$ ";

        protected virtual Command GetCommand(string name)
        {
            return null;
        }

        protected virtual bool RunSpecialCommand(string name, string[] args)
        {
            switch(name.ToLower())
            {
                case "exit":
                    Complete();
                    return true;
                case "clear":
                    Console.Clear();
                    return true;
                case "echo":
                    Console.WriteLine(string.Join(" ", args));
                    return true;
            }
            return false;
        }

        protected override void OnUpdate(float deltaSeconds)
        {
            if (_waitingForCommand && _currentCommand != null)
            {
                _currentCommand.Update(deltaSeconds);
                if(_currentCommand.Completed)
                {
                    CommandCompleted();
                }
                return;
            }

            if(_waitingForInput)
            {
                string input = "";
                if(Console.GetLine(out input))
                {
                    _waitingForInput = false;

                    if (string.IsNullOrWhiteSpace(input)) return;

                    ExecuteLine(input);

                    if(_instructions.Count > 0)
                    {
                        ExecuteNextCommand();
                        return;
                    }
                }
            }
            else
            {
                if(_instructions.Count > 0)
                {
                    ExecuteNextCommand();
                    return;
                }

                Console.Write(this.ShellPrompt);
                _waitingForInput = true;
            }
        }

        private ShellInstruction ParseCommand(string command, string home)
        {
            //This is the list of commands to run in series
            List<string> commands = new List<string>();

            //Parser state
            bool escaping = false;
            bool inQuote = false;
            string current = "";
            bool isFileName = false;

            //output file name
            string fileName = "";

            //Will the game overwrite the specified file with output?
            bool shouldOverwriteOnFileRedirect = false;

            //Length of the input command.
            int cmdLength = command.Length;

            //Iterate through each character in the command.
            for (int i = 0; i < cmdLength; i++)
            {
                //Get the character at the current index.
                char c = command[i];

                //If we're a backslash, parse an escape sequence.
                if (c == '\\')
                {
                    //Ignore escape if we're not in a quote or file.
                    if (!(inQuote || isFileName))
                    {
                        current += c;
                        continue;
                    }
                    //If we're not currently escaping...
                    if (!escaping)
                    {
                        escaping = true;
                        //If we're a filename, append to the filename string.
                        if (isFileName)
                        {
                            fileName += c;
                        }
                        else
                        {
                            current += c;
                        }
                        continue;
                    }
                    else
                    {
                        escaping = false;
                    }
                }
                else if (c == '"')
                {
                    if (!isFileName)
                    {
                        if (!escaping)
                        {
                            inQuote = !inQuote;
                        }
                    }
                }
                if (c == '|' && this.AllowPipes)
                {
                    if (!isFileName)
                    {
                        if (!inQuote)
                        {
                            if (string.IsNullOrWhiteSpace(current))
                            {
                                throw new ShellParseException("unexpected token '|' (pipe)");
                            }
                            commands.Add(current.Trim());
                            current = "";
                            continue;
                        }
                    }
                }
                else if (char.IsWhiteSpace(c))
                {
                    if (isFileName)
                    {
                        if (!escaping)
                        {
                            if (string.IsNullOrWhiteSpace(fileName))
                            {
                                continue;
                            }
                            else
                            {
                                throw new ShellParseException("unexpected whitespace in filename.");
                            }
                        }
                    }
                }
                else if (c == '>' && this.AllowRedirection)
                {
                    if (!isFileName)
                    {
                        isFileName = true;
                        shouldOverwriteOnFileRedirect = true;
                        continue;
                    }
                    else
                    {
                        if (command[i - 1] == '>')
                        {
                            if (!shouldOverwriteOnFileRedirect)
                            {
                                shouldOverwriteOnFileRedirect = false;
                            }
                            else
                            {
                                throw new ShellParseException("unexpected token '>' (redirect) in filename");
                            }
                            continue;
                        }
                    }
                }
                if (isFileName)
                    fileName += c;
                else
                    current += c;
                if (escaping)
                    escaping = false;

            }
            if (inQuote)
            {
                throw new ShellParseException("expected closing quotation mark, got end of command.");
            }
            if (escaping)
            {
                throw new ShellParseException("expected escape sequence, got end of command.");
            }
            if (!string.IsNullOrWhiteSpace(current))
            {
                commands.Add(current.Trim());
                current = "";
            }
            if (!string.IsNullOrWhiteSpace(fileName))
            {
                if (fileName.StartsWith("~"))
                {
                    fileName = fileName.Remove(0, 1);
                    while (fileName.StartsWith("/"))
                    {
                        fileName = fileName.Remove(0, 1);
                    }
                    if (home.EndsWith("/"))
                    {
                        fileName = home + fileName;
                    }
                    else
                    {
                        fileName = home + "/" + fileName;
                    }
                }
            }

            return new ShellInstruction(commands.ToArray(), fileName, shouldOverwriteOnFileRedirect);
        }

        protected void FinishSpecialCommand()
        {
            this.CommandCompleted();
        }

        protected void CommandCompleted()
        {
            if(this._currentConsole is Pipe)
            {
                (_currentConsole as Pipe).WriteRedirect();
            }

            this._lastConsoleContext = this._currentConsole;
            this._currentConsole = null;
            this._instructions.RemoveAt(0);
            this._waitingForCommand = false;
        }

        private void ExecuteNextCommand()
        {
            _waitingForCommand = true;
            _currentConsole = _instructions[0].Console;
            string[] specialArgs = _instructions[0].Arguments.Skip(1).ToArray();
            ConsoleOverride = _currentConsole;
            bool SpecialCommandFound = false;
            if (RunSpecialCommand(_instructions[0].Name, specialArgs))
            {
                CommandCompleted();
                SpecialCommandFound = true; ;
            }
            ConsoleOverride = null;
            if (SpecialCommandFound) return;

            _currentCommand = GetCommand(_instructions[0].Name);

            if (_currentCommand == null)
            {
                _currentConsole.WriteLine("{0}: command not found.", _instructions[0].Name);
                CommandCompleted();
                return;
            }

            _currentCommand.Run(_currentConsole, _instructions[0].Arguments);
        }

        private void ExecuteLine(string text)
        {
            text = text.Trim();

            try
            {
                var instructionData = ParseCommand(text, User.HomeFolder);

                if(!string.IsNullOrWhiteSpace(instructionData.OutputFile))
                {
                    string abs = GetAbsolutePath(instructionData.OutputFile);
                    if(User.FileSystem.DirectoryExists(abs))
                    {
                        Console.WriteLine("{0}: {1}: Directory exists.", CommandName, abs);
                        return;
                    }
                }

                if (instructionData.Commands.Length == 0) return;

                Pipe LastPipe = null;

                int i = 0;

                foreach(var command in instructionData.Commands)
                {
                    bool IsPiping = (i < instructionData.Commands.Length - 1);
                    string home = User.HomeFolder;

                    string[] tokens = this.Tokenize(command, home);

                    if (tokens.Length == 0) continue;

                    string commandName = tokens[0];

                    IConsoleContext intendedConsole = null;

                    if(IsPiping)
                    {
                        Pipe newPipe = new Pipe(LastPipe, Console, null);
                        LastPipe = newPipe;
                        intendedConsole = newPipe;
                    }
                    else
                    {
                        if(!string.IsNullOrWhiteSpace(instructionData.OutputFile))
                        {
                            intendedConsole = new Pipe(LastPipe, Console, null);
                            string abs = GetAbsolutePath(instructionData.OutputFile);
                            (intendedConsole as Pipe).Redirect(abs, instructionData.Overwrites);
                        }
                        else
                        {
                            intendedConsole = new Pipe(LastPipe, Console, Console);
                        }
                    }

                    CommandInstruction instruction = new CommandInstruction(commandName, tokens, intendedConsole);

                    _instructions.Add(instruction);

                    i++;
                }
            }
            catch(ShellParseException ex)
            {
                Console.WriteLine(ex.Message);
            }

            _lastConsoleContext = Console;
        }

        private string[] Tokenize(string command, string home)
        {
            List<string> tokens = new List<string>();
            string current = "";
            bool escaping = false;
            bool inQuote = false;

            int cmdLength = command.Length;

            char[] cmd = command.ToCharArray();

            for (int i = 0; i < cmdLength; i++)
            {
                char c = cmd[i];
                if (c == '\\')
                {
                    if (escaping == false)
                        escaping = true;
                    else
                    {
                        escaping = false;
                        current += c;
                    }
                    continue;
                }
                if (escaping == true)
                {
                    switch (c)
                    {
                        case ' ':
                        case '~':
                        case '"':
                            current += c;
                            break;
                        case 'n':
                            current += "\n";
                            break;
                        case 'r':
                            current += "\r";
                            break;
                        case 't':
                            current += "\t";
                            break;
                        default:
                            throw new ShellParseException("unrecognized escape sequence.");
                    }
                    escaping = false;
                    continue;
                }
                if (c == '~')
                {
                    if (inQuote == false && string.IsNullOrEmpty(current))
                    {
                        current += home;
                        continue;
                    }
                }
                if (char.IsWhiteSpace(c))
                {
                    if (inQuote)
                    {
                        current += c;
                    }
                    else
                    {
                        if (!string.IsNullOrWhiteSpace(current))
                        {
                            tokens.Add(current);
                            current = "";
                        }
                    }
                    continue;
                }
                if (c == '"')
                {
                    inQuote = !inQuote;
                    if (!inQuote)
                    {
                        if (i + 1 < cmdLength)
                        {
                            if (cmd[i + 1] == '"')
                            {
                                throw new ShellParseException("String splice detected. Did you mean to use a literal double-quote (\\\")?");
                            }
                        }
                    }
                    continue;
                }
                current += c;
            }
            if (inQuote)
            {
                throw new ShellParseException("expected ending double-quote, got end of command instead.");
                
            }
            if (escaping)
            {
                throw new ShellParseException("expected escape sequence, got end of command instead.");
            }
            if (!string.IsNullOrEmpty(current))
            {
                tokens.Add(current);
                current = "";
            }
            return tokens.ToArray();
        }
    }
}
