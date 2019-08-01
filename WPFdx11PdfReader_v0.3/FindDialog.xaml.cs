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

namespace WPFdx11PdfReader_v0._3
{
    /// <summary>
    /// Interaction logic for FindDialog.xaml
    /// </summary>
    public partial class FindDialog : Window
    {
        SearchEngine searchEngine;
        public FindDialog()
        {
            InitializeComponent();
            searchEngine = new SearchEngine();
        }

        private void tbFind_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!String.IsNullOrEmpty(tbFind.Text))
            {
                btFindNext.IsEnabled = true;
            }
            else
            {
                btFindNext.IsEnabled = false;
            }
        }

        private void btFind_Clicked(object sender, RoutedEventArgs e)
        {
            bool direction = false;
            if (rbForward.IsChecked == true)
                direction = false;
            else
                direction = true;


            bool whole_word = false;
            if (chbWholeWord.IsChecked == true)
                whole_word = true;
            else
                whole_word = false;

            bool case_sensetive = false;
            if (chbCaseSensetive.IsChecked == true)
                case_sensetive = true;
            else
                case_sensetive = false;

            searchEngine.UpdateSearchFlags(tbFind.Text, direction, whole_word, case_sensetive);
            searchEngine.Run();
        }
    }
}
