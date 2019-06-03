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
        public StoryCharacter(StoryCharacterData data, ContentManager content, List<Exploit> exploits) : base(Guid.NewGuid().ToString(), content)
        {
            Exploits = exploits;

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
        public List<Exploit> Exploits { get; }
    }

    public class StoryCharacterData : AssetBuilder<StoryCharacter>
    {
        public override StoryCharacter Build(ItemContainer items)
        {
            List<Exploit> exploits = new List<Exploit>();
            foreach(var id in this.ExploitIds)
            {
                var exploit = items.GetItem<Exploit>(id);
                if (exploit != null && !exploits.Any(x => x.Id == exploit.Id))
                    exploits.Add(exploit);
            }

            return new StoryCharacter(this, items.Content, exploits);
        }

        public string Name { get; set; }
        public bool UseNameForEmail { get; set; }
        public string EmailAlias { get; set; }
        public bool UseNameForHostname { get; set; }
        public string Hostname { get; set; }
        public bool UseEmailAsUsername { get; set; }
        public string Username { get; set; }
        public List<string> ExploitIds { get; set; }
    }
}
