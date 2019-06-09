using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.FileSystem
{
    [Serializable]
    public class TextFile
    {
        public int Id { get; set; }
        public string Content { get; set; }
    }
}
