using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public abstract class AssetBuilder<T> where T: Asset
    {
        public abstract T Build(ItemContainer items);
    }
}
