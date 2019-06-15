using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.FileSystem;

namespace ThePeacenet.Backend.OS
{
    internal interface IKernel : IContentProvider
    {
        WorldState WorldState { get; }

        Identity Identity { get; }
        Computer Computer { get; }

        void Update(float deltaSeconds);

        SystemContext SystemContext { get; }

        IEnumerable<FileRecord> FileRecords { get; }
        IUserLand GetUserLand(int userId);
        bool Execute(IUserLand user, string program, out IProcess process);
    }
}
