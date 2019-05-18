using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetAttributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
    public class DescriptionAttribute : Attribute
    {
        public string Description { get; private set; }

        public DescriptionAttribute(string description)
        {
            Description = description;
        }
    }
}
