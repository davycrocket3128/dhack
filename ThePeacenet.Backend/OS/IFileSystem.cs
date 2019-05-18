using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.FileSystem;

namespace ThePeacenet.Backend.OS
{
    public interface IKernelFileSystem
    {
        List<FileRecord> Records { get; }
        List<Folder> Folders { get; }
        List<TextFile> TextFiles { get; }
        int NextRecordId { get; }
        int NextTextFileId { get; }
        int NextFolderId { get; }

        Folder GetFolder(string path);
        FileRecord GetFile(string path);
    }

    public interface IFileSystem
    {
        bool CreateDirectory(string path);

        bool DirectoryExists(string path);
        bool FileExists(string path);

        bool Delete(string path, bool recursive = false);

        string[] GetDirectories(string path);
        string[] GetFiles(string path);

        void WriteText(string path, string text);
        string ReadText(string path);

        bool MoveFile(string sourcePath, string destinationPath, bool overwrite);
        bool MoveFolder(string sourcePath, string destinationPath, bool overwrite);

        bool CopyFile(string sourcePath, string destinationPath, bool overwrite);

        void SetFileRecord(string path, FileRecordType type, int id);
        FileRecord GetFileRecord(string path);

        string GetAbsolutePath(string path);
    }
}
