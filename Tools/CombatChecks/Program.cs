using System;
using Eclipse.Tests;
int failed = 0;
for (int i = 0; i < CombatScenarios.Names.Length; i++)
{
    try { CombatScenarios.Run(i); Console.WriteLine("PASS " + CombatScenarios.Names[i]); }
    catch (Exception e) { failed++; Console.WriteLine("FAIL " + CombatScenarios.Names[i] + ": " + e.Message); }
}
Console.WriteLine($"{CombatScenarios.Names.Length - failed}/{CombatScenarios.Names.Length} passed");
return failed == 0 ? 0 : 1;
