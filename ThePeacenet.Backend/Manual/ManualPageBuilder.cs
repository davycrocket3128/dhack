using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Manual
{
    public class ManualPageBuilder
    {
        private ManualPage _page = new ManualPage();

		public string Id { get => _page.Id; set => _page.Id = value; }
        public string Name { get => _page.Name; set => _page.Name = value; }
        public string Description { get => _page.Description; set => _page.Description = value; }
        public string ItemType { get => _page.ItemType; set => _page.ItemType = value; }

        public ManualPage Page => _page;

		public void SetMetadata(string title, string content)
        {
			if(_page.Metadata.Any(x=>x.Title == title))
            {
                _page.Metadata.First(x => x.Title == title).Content = content;
            }
			else
            {
                _page.Metadata.Add(new ManualMetadata
                {
                    Title = title,
                    Content = content
                });
            }
        }
    }
}
