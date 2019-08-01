using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Runtime.InteropServices;
using System.Windows.Interop;


namespace WPFdx11PdfReader_v0._3
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        bool lastVisible;
        TimeSpan lastRender;
        IntPtr m_surface;
        

        BookmarksIO m_bookmarks;

        string m_filepath;
        int m_now_view;

        public MainWindow()
        {
            InitializeComponent();
            this.host.Loaded += new RoutedEventHandler(this.Host_Loaded);
            this.host.SizeChanged += new SizeChangedEventHandler(this.Host_SizeChanged);
            this.KeyUp += new KeyEventHandler(this.Key_Pressed);
            m_bookmarks = new BookmarksIO();
            m_bookmarks.ReadBookmarks();

            //m_bookmarks = new Dictionary<int, MenuItem>();
        }

        private static bool Init()
        {
            bool initSucceeded = NativeMethods.InvokeWithDllProtection(() => NativeMethods.Init()) >= 0;

            if (!initSucceeded)
            {
                MessageBox.Show("Failed to initialize.", "WPF D3D Interop", MessageBoxButton.OK, MessageBoxImage.Error);

                if (Application.Current != null)
                {
                    Application.Current.Shutdown();
                }
            }

            return initSucceeded;
        }

        private static int Render(IntPtr resourcePointer, bool isNewSurface)
        {
            return NativeMethods.InvokeWithDllProtection(() => NativeMethods.Render(resourcePointer, isNewSurface));
        }

        private static void Cleanup()
        {
            NativeMethods.InvokeWithDllProtection(NativeMethods.Cleanup);
        }

        private static void SetLeft(bool aState)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.SetLeft(aState));
        }

        private static void SetRight(bool aState)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.SetRight(aState));
        }

        private static void SetDay(bool aState)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.SetDay(aState));
        }

        private static void SetNight(bool aState)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.SetNight(aState));
        }

        private static void SetReverse()
        {
            NativeMethods.InvokeWithDllProtection(NativeMethods.SetReverse);
        }

        private static void OpenDocument(string filename)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.OpenDocument(filename));
        }

        private static int GetNowPageWidth()
        {
            return NativeMethods.InvokeWithDllProtection(NativeMethods.GetNowPageWidth);
        }

        private static int GetNowPageHeight()
        {
            return NativeMethods.InvokeWithDllProtection(NativeMethods.GetNowPageHeight);
        }

        private static void SaveImage(int percent, string filename)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.SaveImage(percent, filename));
        }

        public static bool Search(string searchstr, int search_page, bool direction, bool case_sensetive)
        {
            return NativeMethods.InvokeWithDllProtection(() => NativeMethods.Search(searchstr, search_page, direction, case_sensetive));
        }

        public static int GetDocSize()
        {
            return NativeMethods.InvokeWithDllProtection(NativeMethods.GetDocSize);
        }

        public static int GetNowPage()
        {
            return NativeMethods.InvokeWithDllProtection(NativeMethods.GetNowPage);
        }

        private static void ViewBookmark(int page_num)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.ViewBookmark(page_num));
        }

        private static void AddBookmark()
        {
            NativeMethods.InvokeWithDllProtection(NativeMethods.AddBookmark);
        }

        private static void RemoveBookmark()
        {
            NativeMethods.InvokeWithDllProtection(NativeMethods.RemoveBookmark);
        }

        public static void AddBookmarksFromFile(int page_num)
        {
            NativeMethods.InvokeWithDllProtection(() => NativeMethods.AddBookmarksFromFile(page_num));
        }

        public static void ClearBookmarks()
        {
            NativeMethods.InvokeWithDllProtection(NativeMethods.ClearBookmarks);
        }

        private void Host_Loaded(object sender, RoutedEventArgs e)
        {
            Init();
            this.InitializeRendering();
        }

        private void InitializeRendering()
        {
            InteropImage.WindowOwner = (new System.Windows.Interop.WindowInteropHelper(this)).Handle;
            InteropImage.OnRender = this.DoRender;

            InteropImage.RequestRender();
        }

        private void DoRender(IntPtr surface, bool isNewSurface)
        {
            m_surface = surface;
            Render(surface, isNewSurface);
        }

        private void Host_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            double dpiScale = 1.0; // default value for 96 dpi

            // determine DPI
            // (as of .NET 4.6.1, this returns the DPI of the primary monitor, if you have several different DPIs)
            var hwndTarget = PresentationSource.FromVisual(this).CompositionTarget as HwndTarget;
            if (hwndTarget != null)
            {
                dpiScale = hwndTarget.TransformToDevice.M11;
            }

            int surfWidth = (int)(host.ActualWidth < 0 ? 0 : Math.Ceiling(host.ActualWidth * dpiScale));
            int surfHeight = (int)(host.ActualHeight < 0 ? 0 : Math.Ceiling(host.ActualHeight * dpiScale));

            // Notify the D3D11Image of the pixel size desired for the DirectX rendering.
            // The D3DRendering component will determine the size of the new surface it is given, at that point.
            InteropImage.SetPixelSize(surfWidth, surfHeight);

            // Stop rendering if the D3DImage isn't visible - currently just if width or height is 0
            // TODO: more optimizations possible (scrolled off screen, etc...)
            bool isVisible = (surfWidth != 0 && surfHeight != 0);
            if (lastVisible != isVisible)
            {
                lastVisible = isVisible;
                if (lastVisible)
                {
                    CompositionTarget.Rendering += CompositionTarget_Rendering;
                }
                else
                {
                    CompositionTarget.Rendering -= CompositionTarget_Rendering;
                }
            }
        }

        void CompositionTarget_Rendering(object sender, EventArgs e)
        {
            RenderingEventArgs args = (RenderingEventArgs)e;

            // It's possible for Rendering to call back twice in the same frame 
            // so only render when we haven't already rendered in this frame.
            if (this.lastRender != args.RenderingTime)
            {
                InteropImage.RequestRender();
                this.lastRender = args.RenderingTime;
            }
        }


        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            this.UninitializeRendering();
        }

        private void UninitializeRendering()
        {
            Cleanup();

            CompositionTarget.Rendering -= this.CompositionTarget_Rendering;
        }

        private void Key_Pressed(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Left:
                    SetLeft(true);
                    break;
                case Key.Right:
                    SetRight(true);
                    break;
                case Key.D:
                    SetDay(true);
                    break;
                case Key.N:
                    SetNight(true);
                    break;
            }
        }


        private static class NativeMethods
        {
            /// <summary>
            /// Variable used to track whether the missing dependency dialog has been displayed,
            /// used to prevent multiple notifications of the same failure.
            /// </summary>
            private static bool errorHasDisplayed;

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int Init();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int Render(IntPtr resourcePointer, bool isNewSurface);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void Cleanup();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetLeft(bool aState);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetRight(bool aState);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetDay(bool aState);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetNight(bool aState);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetReverse();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            public static extern void OpenDocument(string filename);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int GetNowPageWidth();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int GetNowPageHeight();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            public static extern void SaveImage(int percent, string filename);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            public static extern bool Search(string searchstr, int search_page, bool direction, bool case_sensetive);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int GetDocSize();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int GetNowPage();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ViewBookmark(int page_num);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void AddBookmark();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void RemoveBookmark();

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void AddBookmarksFromFile(int page_num);

            [DllImport("D3DVisualization.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ClearBookmarks();

            /// <summary>
            /// Method used to invoke an Action that will catch DllNotFoundExceptions and display a warning dialog.
            /// </summary>
            /// <param name="action">The Action to invoke.</param>
            public static void InvokeWithDllProtection(Action action)
            {
                InvokeWithDllProtection(
                    () =>
                    {
                        action.Invoke();
                        return 0;
                    });
            }

            /// <summary>
            /// Method used to invoke A Func that will catch DllNotFoundExceptions and display a warning dialog.
            /// </summary>
            /// <param name="func">The Func to invoke.</param>
            /// <returns>The return value of func, or default(T) if a DllNotFoundException was caught.</returns>
            /// <typeparam name="T">The return type of the func.</typeparam>
            public static T InvokeWithDllProtection<T>(Func<T> func)
            {
                try
                {
                    return func.Invoke();
                }
                catch (DllNotFoundException e)
                {
                    if (!errorHasDisplayed)
                    {
                        MessageBox.Show("This sample requires:\nManual build of the D3DVisualization project, which requires installation of Windows 10 SDK or DirectX SDK.\n" +
                                        "Installation of the DirectX runtime on non-build machines.\n\n" +
                                        "Detailed exception message: " + e.Message, "WPF D3D11 Interop",
                                        MessageBoxButton.OK, MessageBoxImage.Error);
                        errorHasDisplayed = true;

                        if (Application.Current != null)
                        {
                            Application.Current.Shutdown();
                        }
                    }
                }

                return default(T);
            }
        }

        private void OpenBinding_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            ClearBookmarks();
            ClearBookmarksMenu();

            dlg.FileName = "Document";
            dlg.DefaultExt = ".pdf";
            dlg.Filter = "Documents (.pdf; .xps; .cbz; .epub; .fb2; .zip; .png; .jpeg; .tiff)|*.zip;*.cbz;*.xps;*.epub;*.fb2;*.pdf;*.jpe;*.jpg;*.jpeg;*.jfif;*.tif;*.tiff|PDF Files (.pdf)|*.pdf|XPS Files (.xps)|*.xps|CBZ Files (.cbz;.zip)|*.zip;*.cbz|EPUB Files (.epub)|*.epub|FictionBook 2 Files (.fb2)|*.fb2|Image Files (.png;.jpeg;.tiff)|*.png;*.jpg;*.jpe;*.jpeg;*.jfif;*.tif;*.tiff|All Files|*||";
	
            Nullable<bool> result = dlg.ShowDialog();

            if (result == true)
            {
                m_filepath = dlg.FileName;
                int bookmarks_num = m_bookmarks.CheckBookmarks(m_filepath);
                if (bookmarks_num != 0)
                {
                    for (int i = 0; i < bookmarks_num; i++)
                    {
                        int bookmark_page = m_bookmarks.GetBookmark(i);

                        MenuItem bookmark = new MenuItem();
                        bookmark.Header = "Закладка " + (bookmark_page + 1);
                        bookmark.Click += on_Added_Bookmark_Click;
                        bookmark.GotFocus += on_GotFocus_Bookmark;
                        bookmark.LostFocus += on_LostFocus_Bookmark;


                        m_bookmarks.UpdateBookmarks(bookmark_page, bookmark);
                        Bookmarks.Items.Add(bookmark);
                        AddBookmark();
                        
                    }
                }
                OpenDocument(dlg.FileName);
            }
        }

        private void SaveAsBinding_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            ResolutionDialog dlg = new ResolutionDialog(GetNowPageWidth(), GetNowPageHeight());

            dlg.Owner = this;

            Nullable<bool> result = dlg.ShowDialog();

            if (result == true)
            {
                int percent = dlg.m_percent;

                Microsoft.Win32.SaveFileDialog savedlg = new Microsoft.Win32.SaveFileDialog();
                savedlg.FileName = "Document";
                savedlg.DefaultExt = ".png";
                savedlg.Filter = "Image (.png)|*.png";

                Nullable<bool> saveresult = savedlg.ShowDialog();

                if (saveresult == true)
                {
                    string filename = savedlg.FileName;
                    SaveImage(percent, savedlg.FileName);
                }
            }
        }

        private void FindBinding_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            FindDialog dlg = new FindDialog();

            dlg.ShowDialog();
        }

        private void CloseBinding_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            this.UninitializeRendering();
            this.Close();
        }

        private void Day_NightClick(object sender, RoutedEventArgs e)
        {
            SetReverse();
        }

        private void AddBookmark_Click(object sender, RoutedEventArgs e)
        {
            MenuItem bookmark = new MenuItem();
            bookmark.Header = "Закладка " + (GetNowPage() + 1);
            bookmark.Click += on_Added_Bookmark_Click;
            bookmark.GotFocus += on_GotFocus_Bookmark;
            bookmark.LostFocus += on_LostFocus_Bookmark;
            if (m_bookmarks.AddBookmark(GetNowPage(), bookmark, m_filepath))
            {
                Bookmarks.Items.Add(bookmark);
                AddBookmark();
            }
        }

        private void DeleteBookmark_Click(object sender, RoutedEventArgs e)
        {
            if (m_bookmarks.RemoveBookmark(GetNowPage(), Bookmarks, m_filepath))
                RemoveBookmark();
        }

        private void on_Added_Bookmark_Click(object sender, RoutedEventArgs e)
        {
            ViewBookmark(m_bookmarks.GetClickedBookmark(sender));
        }

        private void on_GotFocus_Bookmark(object sender, RoutedEventArgs e)
        {
            m_now_view = GetNowPage();
            ViewBookmark(m_bookmarks.GetClickedBookmark(sender));
        }

        private void on_LostFocus_Bookmark(object sender, RoutedEventArgs e)
        {
            ViewBookmark(m_now_view);
        }

        private void ClearBookmarksMenu()
        {
            m_bookmarks.ClearBookmarks(Bookmarks);
        }
    }
}
