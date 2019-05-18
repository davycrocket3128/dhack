using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend.AssetAttributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
    public class RamUsageAttribute : Attribute
    {
        public RamUsage RamUsage { get; private set; }

        public RamUsageAttribute(RamUsage usage)
        {
            RamUsage = usage;
        }
    }
}
