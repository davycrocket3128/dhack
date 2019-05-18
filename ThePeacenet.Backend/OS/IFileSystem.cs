using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.OS
{
    public interface IFileSystem
    {
        bool DirectoryExists(string path);
        bool FileExists(string path);

        void CreateDirectory(string path);

        void DeleteFile(string path);
        void DeleteDirectory(string path, bool recursive = false);

        string[] GetFiles(string path);
        string[] GetDirectories(string path);
    }
}
