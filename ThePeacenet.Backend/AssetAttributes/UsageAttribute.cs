using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetAttributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true)]
    public class UsageAttribute : Attribute
    {
        public string Usage { get; private set; }

        public UsageAttribute(string usage)
        {
            Usage = usage;
        }
    }
}
