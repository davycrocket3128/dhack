using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Data
{
    public enum UserType
    {
        Admin,
        Sudoer,
        User
    }

    public class User
    {
        public int Id { get; set; }
        public string Username { get; set; }
        public string Password { get; set; }
        public ConsoleColor UserColor { get; set; }
        public UserType UserType { get; set; } = UserType.User;
    }
}
