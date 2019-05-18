using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.OS
{
    public interface IKernel : IContentProvider
    {
        WorldState WorldState { get; }

        IUserLand GetUserLand(int userId);
        bool Execute(IUserLand user, string program, out IProcess process);
    }
}
