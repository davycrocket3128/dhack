using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetAttributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
    public class UnlockedByDefaultAttribute : Attribute
    {
    }
}
