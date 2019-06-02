using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui;

namespace ThePeacenet.Backend.AssetTypes
{
    public class ProgramGui
    {
        public string EventHandlerClass { get; set; }

        [ContentSerializer(CollectionItemName = "Event", Optional = true)]
        public List<Event> WindowEvents { get; set; } = new List<Event>();

        public ControlElement Content { get; set; }
    }

    public class Event
    {
        public string Name { get; set; }
        public string Handler { get; set; }
    }

    public class ControlElement
    {
        [ContentSerializer(Optional = true)]
        public string Name { get; set; } = Guid.NewGuid().ToString();
        public string Type { get; set; }

        [ContentSerializer(CollectionItemName = "Property", Optional = true)]
        public List<ControlProperty> Properties { get; set; } = new List<ControlProperty>();

        [ContentSerializer(CollectionItemName = "Event", Optional = true)]
        public List<Event> Events { get; set; } = new List<Event>();

        [ContentSerializer(CollectionItemName = "Child", Optional = true)]
        public List<ControlElement> Children { get; set; } = new List<ControlElement>();
    }
}
