using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;

namespace ThePeacenet.Backend.OS
{
    public interface IContentProvider
    {
        IEnumerable<CommandAsset> Commands { get; }
    }
}
