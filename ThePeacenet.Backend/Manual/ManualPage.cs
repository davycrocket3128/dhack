using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Manual
{
    public sealed class ManualMetadata
    {
        public string Title { get; set; }
        public string Content { get; set; }
    }

    public sealed class ManualPage
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string ItemType { get; set; }
        public List<ManualMetadata> Metadata { get; set; }
    }
}
