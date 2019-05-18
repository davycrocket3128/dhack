using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.FileSystem
{
    public class Folder
    {
        public int Id { get; set; }
        public string Name { get; set; }
        public List<int> SubFolders { get; set; } = new List<int>();
        public bool IsReadOnly { get; set; }
        public List<int> Files { get; set; } = new List<int>();
        public int Parent { get; set; } = -1;
    }
}
