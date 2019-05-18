using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend
{
    public static class ReflectionTools
    {
        public static Type[] GetAll<T>()
        {
            List<Type> types = new List<Type>();

            foreach(var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                foreach(var type in assembly.GetTypes())
                {
                    if(typeof(T).IsAssignableFrom(type) && !type.IsAbstract)
                    {
                        types.Add(type);
                    }
                }
            }

            return types.ToArray();
        }
    }
}
