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
using System.Windows.Shapes;
using System.Text.RegularExpressions;

namespace WPFdx11PdfReader_v0._3
{
    /// <summary>
    /// Interaction logic for ResolutionDialog.xaml
    /// </summary>
    public partial class ResolutionDialog : Window
    {
        public int m_percent { get; private set; }
        int m_width;
        int m_height;
        bool m_edit_text;
        int m_start_height;
        int m_start_width;

        public ResolutionDialog(int width, int height)
        {
            InitializeComponent();
            m_percent = 100;
            m_width = width;
            m_height = height;
            tbPercent.Text = m_percent.ToString();
            tbWidth.Text = m_width.ToString();
            tbHeight.Text = m_height.ToString();
            m_edit_text = true;
            m_start_height = m_height;
            m_start_width = m_width;

        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
        }

        private void cancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
        }

        private void PreviewEventHandler(object sender, TextCompositionEventArgs e)
        {
            Regex _regex = new Regex("[^0-9]+");
            e.Handled = _regex.IsMatch(e.Text);
        }

        private void tbPercentEventHandler(object sender, TextChangedEventArgs e)
        {
            if (m_edit_text)
            {
                m_percent = Convert.ToInt32(tbPercent.Text);
                m_height = (int)((float)m_start_height * (float)m_percent / 100.0f);
                m_width = (int)((float)m_start_width * (float)m_percent / 100.0f);

                m_edit_text = false;
                tbHeight.Text = m_height.ToString();
                tbWidth.Text = m_width.ToString();
                m_edit_text = true;
                tbPercent.SelectionStart = tbPercent.Text.Length;
            }
        }

        private void tbHeightEventHandler(object sender, TextChangedEventArgs e)
        {
            if (m_edit_text)
            {
                int new_resolution = Convert.ToInt32(tbHeight.Text);
                m_percent = (int)(((float)new_resolution * 100.0f) / (float)m_start_height);
                m_width = (int)((float)m_start_width * (float)m_percent / 100.0f);
                m_height = new_resolution;

                m_edit_text = false;
                tbPercent.Text = m_percent.ToString();
                tbWidth.Text = m_width.ToString();
                m_edit_text = true;
                tbHeight.SelectionStart = tbHeight.Text.Length;
            }
        }

        private void tbWidthEventHandler(object sender, TextChangedEventArgs e)
        {
            if (m_edit_text)
            {
                int new_resolution = Convert.ToInt32(tbWidth.Text);
                m_percent = (int)(((float)new_resolution * 100.0f) / (float)m_start_width);
                m_height = (int)((float)m_start_height * (float)m_percent / 100.0f);
                m_width = new_resolution;

                m_edit_text = false;
                tbPercent.Text = m_percent.ToString();
                tbHeight.Text = m_height.ToString();
                m_edit_text = true;
                tbWidth.SelectionStart = tbWidth.Text.Length;
            }
        }
    }
}
