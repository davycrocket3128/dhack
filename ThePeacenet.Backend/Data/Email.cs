using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Data
{
    public class Email
    {
        public int Id { get; set; }
        public int From { get; set; }
        public List<int> To { get; set; } = new List<int>();
        public int InReplyTo { get; set; }
        public DateTime Sent { get; set; } = DateTime.Now;
        public string Subject { get; set; }
        public string Message { get; set; }
        public string MissionId { get; set; }
    }
}
