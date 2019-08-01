using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace WPFdx11PdfReader_v0._3
{
    class SearchEngine
    {
        string m_searchstr;
        string m_prev_search;
        bool m_direction;
        bool m_whole_word;
        bool m_case_sensetive;

        bool m_forward_search;
        int m_search_page;
        bool m_last;
        bool m_first;
        bool m_not_found;
        int m_size;

        public SearchEngine()
        {
            m_searchstr = null;
            m_prev_search = null;

            m_search_page = 0;
            m_last = false;
            m_first = true;
            m_not_found = true;
            m_size = MainWindow.GetDocSize();
        }

        public void UpdateSearchFlags(string searchstr, bool direction, bool whole_word, bool case_sensetive)
        {
            m_searchstr = searchstr;
            m_direction = direction;
            m_whole_word = whole_word;
            m_case_sensetive = case_sensetive;

            if (!direction)
            {
                m_forward_search = true;
            }
            else
            {
                m_forward_search = false;
            }

            m_size = MainWindow.GetDocSize();
        }


        public void Run()
        {
            InitFlags();
            if (!m_direction)
            {
                ForwardSearch();
            }
            else
            {
                BackwardSearch();
            }
            m_prev_search = m_searchstr;
        }

        void InitFlags()
        {
            if (m_whole_word)
            {
                string spacestr = " ";
                m_searchstr = spacestr + m_searchstr + spacestr;
            }

            if (!m_case_sensetive)
            {
                m_searchstr.ToLower();
            }
        }

        void ForwardSearch()
        {
           
                if (m_forward_search == false)
                    m_search_page++;
                m_forward_search = true;
            if (m_searchstr != m_prev_search && (m_search_page != 0 || m_last == true))
            {
                m_search_page = 0;
                m_last = false;
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                m_search_page++;
                m_first = false;
            }
            else if (m_searchstr == m_prev_search && m_search_page == 0)
            {
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                m_search_page++;
                m_first = false;
                if (m_not_found)
                {
                    m_search_page = 0;
                    MessageBox.Show("Не найдено " + m_searchstr, "Поиск");
                }
            }
            else if (m_search_page != m_size - 1)
            {
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                m_search_page++;
                m_first = false;
            }
            else if (m_search_page == m_size - 1)
            {
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                m_search_page = 0;
                m_last = true;
                m_first = true;
            }
            else
            {
                m_search_page++;
                m_first = false;
            }
            
        }

        void BackwardSearch()
        {
            if (m_forward_search)
            {
                m_search_page--;
                m_forward_search = false;
                if (m_search_page == -1)
                {
                    m_search_page = m_size - 1;
                    m_last = false;
                    m_first = false;
                }
            }
            if (m_search_page == 0 && m_first == true)
            {
                MessageBox.Show("Достигнуто начало документа.", "Поиск");
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                if (m_not_found)
                {
                    m_search_page = 0;
                    MessageBox.Show("Не найдено " + m_searchstr, "Поиск");
                }
            }
            else if (m_search_page != 0)
            {
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                m_search_page--;
                m_last = false;
            }
            else if (m_search_page == 0 && m_first == false)
            {
                if (MainWindow.Search(m_searchstr, m_search_page, m_direction, m_case_sensetive))
                {
                    m_not_found = false;
                }
                m_search_page = 0;
                m_last = false;
                m_first = true;
            }
            else
                m_search_page--;
        }

    }
}
