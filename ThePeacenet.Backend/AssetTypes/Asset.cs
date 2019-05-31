using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public class Asset
    {
        internal Asset(string id)
        {
            Id = id;
        }

        public string Id { get; }
    }
}
