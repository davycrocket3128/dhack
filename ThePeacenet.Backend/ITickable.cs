﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend
{
    public interface ITickable
    {
        void Update(float deltaSeconds);
    }
}