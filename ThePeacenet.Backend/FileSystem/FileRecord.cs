using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.FileSystem
{
    public enum FileRecordType
    {
        Text,
        Program,
        Command,
        Exploit,
        Payload,
        Picture,
        CryptoWallet
    }

    [Serializable]
    public class FileRecord
    {
        public int Id { get; set; }
        public string Name { get; set; }
        public FileRecordType RecordType { get; set; }
        public int ContentId { get; set; }
    }
}
