using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public class StoryCharacter : Asset
    {
        public StoryCharacter(StoryCharacterData data, ContentManager content) : base(Guid.NewGuid().ToString(), content)
        {
            Name = data.Name;
            UseNameForEmail = data.UseNameForEmail;
            EmailAlias = data.EmailAlias;
            UseNameForHostname = data.UseNameForHostname;
            Hostname = data.Hostname;
            UseEmailAsUsername = data.UseEmailAsUsername;
            Username = data.Username;
        }

        public string Name { get; }
        public bool UseNameForEmail { get; }
        public string EmailAlias { get; }
        public bool UseNameForHostname { get; }
        public string Hostname { get; }
        public bool UseEmailAsUsername { get; }
        public string Username { get; }
    }

    public class StoryCharacterData : AssetBuilder<StoryCharacter>
    {
        public override StoryCharacter Build(ItemContainer items)
        {
            return new StoryCharacter(this, items.Content);
        }

        public string Name { get; set; }
        public bool UseNameForEmail { get; set; }

        [ContentSerializer(Optional = true)]
        public string EmailAlias { get; set; } = "";

        public bool UseNameForHostname { get; set; }

        [ContentSerializer(Optional = true)]
        public string Hostname { get; set; }

        public bool UseEmailAsUsername { get; set; }

        [ContentSerializer(Optional = true)]
        public string Username { get; set; }
    }
}
