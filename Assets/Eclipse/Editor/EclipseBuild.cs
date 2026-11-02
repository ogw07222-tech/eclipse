using System;
using System.IO;
using UnityEditor;
using UnityEditor.Build.Reporting;

namespace Eclipse.Editor
{
    public static class EclipseBuild
    {
        public const string Scene = "Assets/Scenes/Main.unity";
        [MenuItem("ECLIPSE/Open Main Scene")]
        public static void OpenMain() { UnityEditor.SceneManagement.EditorSceneManager.OpenScene(Scene); }
        [MenuItem("ECLIPSE/Build Android APK")]
        public static void AndroidApk()
        {
            string output = Environment.GetEnvironmentVariable("ECLIPSE_BUILD_PATH") ?? "Builds/Android/ECLIPSE.apk";
            Directory.CreateDirectory(Path.GetDirectoryName(output));
            EditorUserBuildSettings.buildAppBundle = false;
            Build(new BuildPlayerOptions { scenes = new[] { Scene }, locationPathName = output, target = BuildTarget.Android, options = BuildOptions.Development });
        }
        [MenuItem("ECLIPSE/Build Windows")]
        public static void Windows()
        {
            Directory.CreateDirectory("Builds/Windows");
            Build(new BuildPlayerOptions { scenes = new[] { Scene }, locationPathName = "Builds/Windows/ECLIPSE.exe", target = BuildTarget.StandaloneWindows64, options = BuildOptions.None });
        }
        private static void Build(BuildPlayerOptions options)
        {
            var report = BuildPipeline.BuildPlayer(options);
            if (report.summary.result != BuildResult.Succeeded) throw new InvalidOperationException("ECLIPSE build failed: " + report.summary.result);
        }
    }
}
