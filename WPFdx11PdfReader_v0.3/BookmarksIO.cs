using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Controls;
using System.Windows;

namespace WPFdx11PdfReader_v0._3
{
    class BookmarksIO
    {
        string m_filepath;
        string m_bookmarks_path;
        Dictionary<int, MenuItem> m_bookmarks;
        Dictionary<string, List<int>> m_bookmarks_file;


        public BookmarksIO()
        {
            m_bookmarks_path = "bookmarks.bin";
            m_bookmarks = new Dictionary<int, MenuItem>();
            m_bookmarks_file = new Dictionary<string, List<int>>();
        }
        public void SetFilePath(string filepath)
        {
            m_filepath = filepath;
        }

        public void WriteBookmarks()
        {
            try
            {
                using (BinaryWriter writer = new BinaryWriter(File.Open("bookmarks.bin", FileMode.Create)))
                {
                    var keys = new List<string>(m_bookmarks_file.Keys);
                    for (int i = 0; i < m_bookmarks_file.Count(); i++)
                    {
                        writer.Write(keys.ElementAt(i));
                        writer.Write(m_bookmarks_file[keys.ElementAt(i)].Count());

                        for (int j = 0; j < m_bookmarks_file[keys.ElementAt(i)].Count(); j++)
                        {
                            writer.Write(m_bookmarks_file[keys.ElementAt(i)].ElementAt(j));
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "error");
            }
        }

        public void ReadBookmarks()
        {
            try
            {
                using (BinaryReader reader = new BinaryReader(File.Open(m_bookmarks_path, FileMode.OpenOrCreate)))
                {
                    while (reader.PeekChar() > -1)
                    {
                        string path = reader.ReadString();
                        int data_size = reader.ReadInt32();
                        m_bookmarks_file.Add(path, new List<int>());
                        for (int i = 0; i < data_size; i++)
                        {
                            m_bookmarks_file[path].Add(reader.ReadInt32());
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "error");
            }
        }

        public bool AddBookmark(int key, MenuItem value, string path)
        {
            if (!m_bookmarks.ContainsKey(key))
            {
                m_bookmarks.Add(key, value);

                var new_bookmarks = new List<int>(m_bookmarks.Keys);
                if (m_bookmarks_file.ContainsKey(path))
                {
                    m_bookmarks_file[path] = new_bookmarks;
                }
                else
                {
                    m_bookmarks_file.Add(path, new_bookmarks);
                }
                WriteBookmarks();
                return true;
            }
            return false;
        }


        public bool RemoveBookmark(int key, MenuItem main_menu_item, string path)
        {
            if(m_bookmarks.ContainsKey(key))
            {
                main_menu_item.Items.Remove(m_bookmarks[key]);
                m_bookmarks.Remove(key);

                if (m_bookmarks.Count != 0)
                {
                    var new_bookmarks = new List<int>(m_bookmarks.Keys);
                    if (m_bookmarks_file.ContainsKey(path))
                    {
                        m_bookmarks_file[path] = new_bookmarks;
                    }
                    else
                    {
                        m_bookmarks_file.Add(path, new_bookmarks);
                    }
                }
                else
                {
                    if (m_bookmarks_file.ContainsKey(path))
                        m_bookmarks_file.Remove(path);
                }
                WriteBookmarks();
                return true;
            }
            return false;
        }

        public void ClearBookmarks(MenuItem main_menu_item)
        {
            foreach (var items in m_bookmarks)
            {
                main_menu_item.Items.Remove(items.Value);
            }
        }
        public int GetClickedBookmark(object sender)
        {
            return m_bookmarks.Where(kvp => kvp.Value == sender).Select(kvp => kvp.Key).FirstOrDefault();
        }

        public int CheckBookmarks(string filepath)
        {
            if (m_bookmarks_file.ContainsKey(filepath))
            {
                m_filepath = filepath;
                
                m_bookmarks.Clear();

                for (int i=0; i < m_bookmarks_file[filepath].Count; i++)
                {
                    MainWindow.AddBookmarksFromFile(m_bookmarks_file[filepath].ElementAt(i));
                }

                return m_bookmarks_file[filepath].Count;
            }
            else
                return 0;
                
        }

        public int GetBookmark(int index)
        {
            return m_bookmarks_file[m_filepath].ElementAt(index);
        }

        public void UpdateBookmarks(int key, MenuItem value)
        {
            m_bookmarks.Add(key, value);
        }
    }
}
