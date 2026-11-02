using System.Collections;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.SceneManagement;
using UnityEngine.TestTools;
using UnityEngine.UI;

namespace Eclipse.Tests
{
    public sealed class SceneSmokeTests
    {
        [UnityTest]
        public IEnumerator MainSceneStartsAndRestartsWithoutDuplicatePresentation()
        {
            yield return SceneManager.LoadSceneAsync("Main");
            yield return null;
            var game = Object.FindFirstObjectByType<EclipseGame>();
            Assert.That(game, Is.Not.Null);
            Assert.That(game.Simulation.Foes, Is.EqualTo(4));
            Assert.That(game.GetComponentsInChildren<TouchControl>(true).Length, Is.EqualTo(5));
            foreach (var image in game.GetComponentsInChildren<RawImage>(true)) Assert.That(image.texture, Is.Not.Null, image.name);
            game.Simulation.DamagePlayer(1000);
            yield return null;
            Assert.That(game.Simulation.Ended, Is.True);
            game.Restart(); game.Restart();
            yield return null;
            Assert.That(game.Simulation.Player.Health, Is.EqualTo(100));
            Assert.That(game.Simulation.Foes, Is.EqualTo(4));
            Assert.That(game.GetComponentsInChildren<EventSystem>(true).Length, Is.EqualTo(1));
            Assert.That(game.GetComponentsInChildren<Canvas>(true).Length, Is.EqualTo(1));
            Assert.That(game.Simulation.Projectiles.Count, Is.EqualTo(0));
            LogAssert.NoUnexpectedReceived();
        }

        [UnityTest]
        public IEnumerator TouchButtonOwnsItsPointerAndReleasesOnDisable()
        {
            var owner = new GameObject("Touch ownership test", typeof(RectTransform));
            var button = owner.AddComponent<TouchControl>();
            int presses = 0, releases = 0;
            button.PressChanged = value => { if (value) presses++; else releases++; };
            var first = new PointerEventData(null) { pointerId = 11 };
            var second = new PointerEventData(null) { pointerId = 22 };
            button.OnPointerDown(first); button.OnPointerDown(second); button.OnPointerUp(second);
            Assert.That(presses, Is.EqualTo(1)); Assert.That(releases, Is.EqualTo(0));
            owner.SetActive(false);
            Assert.That(releases, Is.EqualTo(1));
            Object.Destroy(owner);
            yield return null;
        }
    }
}
