using System;

namespace ThePeacenet.Backend
{
    public static class RandomExtensions
{
    public static float NextFloat(this Random rng, float min, float max)
    {
        var range = (max - min);
        var value = (float)rng.NextDouble() * range;
        return value + min;
    }
}
}
