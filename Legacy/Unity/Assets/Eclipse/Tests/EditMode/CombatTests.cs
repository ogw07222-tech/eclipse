using NUnit.Framework;
namespace Eclipse.Tests
{
    public sealed class CombatTests
    {
        [TestCase(0)] [TestCase(1)] [TestCase(2)] [TestCase(3)] [TestCase(4)]
        [TestCase(5)] [TestCase(6)] [TestCase(7)] [TestCase(8)] [TestCase(9)]
        [TestCase(10)] [TestCase(11)] [TestCase(12)] [TestCase(13)] [TestCase(14)]
        public void SourceGameplayIsPreserved(int scenario) { CombatScenarios.Run(scenario); }
    }
}
