using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend
{
    public interface IConsoleContext
    {
        UserContext User { get; }

        void Write(string text);
        void Write(string format, params object[] args);
        void WriteLine(string text);
        void WriteLine(string format, params object[] args);
        void OverWrite(string text);
        void OverWrite(string format, params object[] args);

        bool GetLine(out string text);

        void Clear();

        string WorkingDirectory { get; set; }
    }
}
