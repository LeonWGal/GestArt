using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
[assembly: AssemblyTitle("GestArt Ultimate Portable")]
[assembly: AssemblyDescription("Portable launcher for GestArt Ultimate")]
[assembly: AssemblyCompany("LeonWGal")]
[assembly: AssemblyProduct("GestArt Ultimate")]
[assembly: AssemblyVersion("1.8.0.0")]
[assembly: AssemblyFileVersion("1.8.0.0")]
internal static class PortableLauncher
{
    private const string Version = "1.8.0";
    private const string PayloadHash = "__PAYLOAD_SHA256__";
    private static readonly string CacheRoot = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "LeonWGal", "GestArtUltimate", "portable-runtime");
    private static readonly bool Russian = System.Globalization.CultureInfo.CurrentUICulture.TwoLetterISOLanguageName == "ru";
    private static int exitCode = 1;
    private static string Text(string en, string ru) { return Russian ? ru : en; }
    [STAThread]
    private static int Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        bool silent = Environment.GetEnvironmentVariable("GESTART_PORTABLE_SILENT") == "1";
        using (var window = new Form())
        {
            window.Text = "GestArt Ultimate Portable " + Version;
            window.Width = 420; window.Height = 142;
            window.FormBorderStyle = FormBorderStyle.FixedDialog;
            window.StartPosition = FormStartPosition.CenterScreen;
            window.MaximizeBox = false; window.MinimizeBox = false; window.ControlBox = false;
            var label = new Label { Left = 20, Top = 18, Width = 375, Height = 36, Text = Text("Preparing GestArt Ultimate...", "Подготовка GestArt Ultimate...") };
            var progress = new ProgressBar { Left = 20, Top = 61, Width = 365, Height = 18, Style = ProgressBarStyle.Marquee };
            window.Controls.Add(label); window.Controls.Add(progress);
            window.Shown += delegate {
                if (silent) window.Hide();
                Task.Factory.StartNew(delegate {
                    try
                    {
                        string package = PrepareRuntime();
                        var forwarded = new List<string>(args);
                        bool explicitProfile = Array.Exists(args, a => a == "--data-dir" || a.StartsWith("--data-dir=", StringComparison.Ordinal));
                        if (!explicitProfile)
                        {
                            string data = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "GestArtUltimate-data");
                            Directory.CreateDirectory(data);
                            forwarded.Insert(0, data); forwarded.Insert(0, "--data-dir");
                        }
                        var process = Process.Start(new ProcessStartInfo {
                            FileName = Path.Combine(package, "bin", "GestArtUltimate.exe"),
                            Arguments = JoinArguments(forwarded), WorkingDirectory = package, UseShellExecute = false
                        });
                        if (process == null) throw new IOException("Cannot start GestArt Ultimate.");
                        window.BeginInvoke(new Action(window.Hide));
                        using (process) { process.WaitForExit(); exitCode = process.ExitCode; }
                    }
                    catch (Exception error)
                    {
                        if (silent) Console.Error.WriteLine(error.ToString());
                        else window.Invoke(new Action(delegate {
                            MessageBox.Show(window, Text("GestArt could not start. Move the portable EXE to a writable folder and try again.\n\n", "Не удалось запустить GestArt. Переместите portable EXE в доступную для записи папку и повторите попытку.\n\n") + error.Message, "GestArt Ultimate", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }));
                        exitCode = 1;
                    }
                    finally { window.BeginInvoke(new Action(window.Close)); }
                });
            };
            Application.Run(window);
        }
        return exitCode;
    }
    private static string PrepareRuntime()
    {
        Directory.CreateDirectory(CacheRoot);
        string target = Path.Combine(CacheRoot, Version + "-" + PayloadHash.Substring(0, 16));
        using (var mutex = new Mutex(false, "Local\\GestArtUltimatePortable-" + PayloadHash.Substring(0, 16)))
        {
            bool acquired = false;
            try
            {
                try { acquired = mutex.WaitOne(TimeSpan.FromMinutes(5)); }
                catch (AbandonedMutexException) { acquired = true; }
                if (!acquired) throw new TimeoutException("Another portable launch is still preparing its files.");
                string marker = Path.Combine(target, ".complete");
                if (File.Exists(marker) && File.ReadAllText(marker) == PayloadHash && File.Exists(Path.Combine(target, "bin", "GestArtUltimate.exe")) && File.Exists(Path.Combine(target, "bin", "Qt6Core.dll"))) return target;
                string staging = Path.Combine(CacheRoot, ".extract-" + Guid.NewGuid().ToString("N"));
                Directory.CreateDirectory(staging);
                try
                {
                    using (Stream payload = Assembly.GetExecutingAssembly().GetManifestResourceStream("payload.zip"))
                    {
                        if (payload == null) throw new IOException("The portable payload is missing.");
                        string digest;
                        using (var sha = SHA256.Create()) digest = BitConverter.ToString(sha.ComputeHash(payload)).Replace("-", "").ToLowerInvariant();
                        if (digest != PayloadHash) throw new IOException("The portable payload checksum does not match. Download the file again.");
                        payload.Position = 0;
                        using (var archive = new ZipArchive(payload, ZipArchiveMode.Read))
                        {
                            string root = Path.GetFullPath(staging) + Path.DirectorySeparatorChar;
                            foreach (var entry in archive.Entries)
                            {
                                string destination = Path.GetFullPath(Path.Combine(staging, entry.FullName.Replace('/', Path.DirectorySeparatorChar)));
                                if (!destination.StartsWith(root, StringComparison.OrdinalIgnoreCase)) throw new IOException("Invalid archive entry.");
                                if (String.IsNullOrEmpty(entry.Name)) Directory.CreateDirectory(destination);
                                else
                                {
                                    Directory.CreateDirectory(Path.GetDirectoryName(destination));
                                    using (var source = entry.Open())
                                    using (var output = new FileStream(destination, FileMode.CreateNew, FileAccess.Write)) source.CopyTo(output);
                                }
                            }
                        }
                    }
                    if (!File.Exists(Path.Combine(staging, "bin", "GestArtUltimate.exe"))) throw new IOException("The application executable is missing.");
                    File.WriteAllText(Path.Combine(staging, ".complete"), PayloadHash);
                    if (Directory.Exists(target)) RemoveCacheDirectory(target);
                    Directory.Move(staging, target);
                }
                finally { if (Directory.Exists(staging)) RemoveCacheDirectory(staging); }
                return target;
            }
            finally { if (acquired) mutex.ReleaseMutex(); }
        }
    }
    private static void RemoveCacheDirectory(string path)
    {
        string full = Path.GetFullPath(path);
        string root = Path.GetFullPath(CacheRoot) + Path.DirectorySeparatorChar;
        if (!full.StartsWith(root, StringComparison.OrdinalIgnoreCase)) throw new IOException("Invalid runtime cache path.");
        Directory.Delete(full, true);
    }
    private static string JoinArguments(IEnumerable<string> args)
    {
        var result = new List<string>();
        foreach (string arg in args)
        {
            if (arg.Length > 0 && arg.IndexOfAny(new[] { ' ', '\t', '\n', '\v', '"' }) < 0) { result.Add(arg); continue; }
            var value = new StringBuilder("\""); int slashes = 0;
            foreach (char c in arg)
            {
                if (c == '\\') { slashes++; continue; }
                if (c == '"') value.Append('\\', slashes * 2 + 1).Append(c);
                else value.Append('\\', slashes).Append(c);
                slashes = 0;
            }
            value.Append('\\', slashes * 2).Append('"'); result.Add(value.ToString());
        }
        return String.Join(" ", result);
    }
}
