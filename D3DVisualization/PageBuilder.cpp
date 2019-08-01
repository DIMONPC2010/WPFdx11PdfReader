#include "PageBuilder.h"

PageBuilder::PageBuilder()
{
	Page p;
	p.RGB = new unsigned char[4];
	for(int i = 0; i < 4; i++)
		p.RGB[i] = (unsigned char)0;

	p.width = 1;
	p.height = 1;
	p.page_num = 0;

	for (int i = 0; i < PAGE_NUM; i++)
	{
		m_pageDeque.push_back(p);
		p.page_num++;
	}
}

PageBuilder::PageBuilder(std::wstring filename)
{
	m_reader = std::make_shared<PdfReader>();
	m_filename = filename;
	if (m_reader->ReadPdfDocument(filename, 0))
	{
		m_size = m_reader->GetDocumentSize();
		this->GetStartPages();
	}
}

void PageBuilder::GetStartPages()
{
	Page p;
	p.RGB = m_reader->GetUNORMPage();
	p.width = m_reader->width();
	p.height = m_reader->height();
	p.page_num = 0;
	m_pageDeque.push_front(p);
	m_pageDeque.push_front(p);
	m_pageDeque.push_front(p);

	int page_num = 0;
	page_num = PAGE_NUM - 3;
	if (m_reader->GetDocumentSize() < page_num)
	{
		for (int i = 0; i < m_reader->GetDocumentSize(); i++)
		{
			if (m_reader->ReadPdfDocument(m_filename, i))
			{
				p.RGB = m_reader->GetUNORMPage();
				p.width = m_reader->width();
				p.height = m_reader->height();
				p.page_num = i;
				m_pageDeque.push_back(p);
			}
		}

		for(int i = m_reader->GetDocumentSize(); i < page_num; i++)
			m_pageDeque.push_back(p);
	
	}
	else
	{
		for (int i = 0; i < page_num; i++)
		{
			if (m_reader->ReadPdfDocument(m_filename, i))
			{
				p.RGB = m_reader->GetUNORMPage();
				p.width = m_reader->width();
				p.height = m_reader->height();
				p.page_num = i;
				m_pageDeque.push_back(p);
			}
		}
	}
}

unsigned char *PageBuilder::GetPage(int page_num)
{
		return m_pageDeque[page_num].RGB;
}
bool PageBuilder::SearchOnPageForward(int page_num, std::wstring searchtext)
{
	if (page_num == 0)
	{
		if (m_reader->ReadPdfDocument(m_filename, page_num))
		{
			if (m_reader->SearchText(page_num, searchtext))
			{

				Page p;

				p.RGB = m_reader->GetUNORMPage();
				p.width = m_reader->width();
				p.height = m_reader->height();
				p.page_num = page_num;
				m_pageDeque.at(3) = p;

				return true;
			}
			
		}
	}
	else
	{
		if (m_reader->ReadPdfDocument(m_filename, page_num))
		{
			if (m_reader->SearchText(page_num, searchtext))
			{

				Page p;

				p.RGB = m_reader->GetUNORMPage();
				p.width = m_reader->width();
				p.height = m_reader->height();
				p.page_num = page_num;
				while (m_pageDeque.at(3).page_num != page_num)
					GetNext();

				m_pageDeque.at(3) = p;

				return true;
			}
		}
	}

	return false;
}

bool PageBuilder::SearchOnPageBackward(int page_num, std::wstring searchtext)
{
	if (m_reader->ReadPdfDocument(m_filename, page_num))
	{
		if (m_reader->SearchText(page_num, searchtext))
		{

			Page p;

			p.RGB = m_reader->GetUNORMPage();
			p.width = m_reader->width();
			p.height = m_reader->height();
			p.page_num = page_num;
			while (m_pageDeque.at(3).page_num != page_num)
				GetPrevious();

			m_pageDeque.at(3) = p;
			return true;
		}
	}

	return false;
}

void PageBuilder::SetCaseSensitiveSearch(bool aState)
{
	m_reader->SetCaseSensitiveSearch(aState);
}

int PageBuilder::size()
{
	return m_size;
}

int PageBuilder::width(int page_num)
{
		return m_pageDeque[page_num].width;
}

int PageBuilder::height(int page_num)
{
		return m_pageDeque[page_num].height;
}

int PageBuilder::NowWidth()
{
	return m_pageDeque[3].width;
}

int PageBuilder::NowHeight()
{
	return m_pageDeque[3].height;
}

void PageBuilder::GetNext()
{
	if (m_pageDeque.back().page_num < m_size - 1)
	{
		m_pageDeque.pop_front();

		Page p;
		int lastPage = m_pageDeque.back().page_num;
		if (m_reader->ReadPdfDocument(m_filename, lastPage + 1))
		{
			p.RGB = m_reader->GetUNORMPage();
			p.width = m_reader->width();
			p.height = m_reader->height();
			p.page_num = lastPage + 1;
			m_pageDeque.push_back(p);
		}
	}
	else
	{
		m_pageDeque.pop_front();
		m_pageDeque.push_back(m_pageDeque.back());
	}
}

void PageBuilder::GetPrevious()
{
	if (m_pageDeque[2].page_num == 0 && m_pageDeque[3].page_num != 0 ||
		m_pageDeque[1].page_num == 0 && m_pageDeque[2].page_num != 0 ||
		m_pageDeque[0].page_num == 0 && m_pageDeque[1].page_num != 0)
	{
		m_pageDeque.push_front(m_pageDeque.front());
		m_pageDeque.pop_back();
	}
	else
	{
		Page p;
		int firstPage = m_pageDeque.front().page_num;
		if (m_reader->ReadPdfDocument(m_filename, firstPage - 1))
		{
			p.RGB = m_reader->GetUNORMPage();
			p.width = m_reader->width();
			p.height = m_reader->height();
			p.page_num = firstPage - 1;
			m_pageDeque.push_front(p);
		}
	}
}

int PageBuilder::NowView()
{
	return m_pageDeque.at(3).page_num;
}

bool PageBuilder::Bookmark(int page_num)
{
	return m_pageDeque[page_num].bookmark;
}

void PageBuilder::SetBookmark(bool aState)
{
	m_pageDeque.at(3).bookmark = aState;
}

void PageBuilder::CheckBookmarks(std::list<int> bookmarks)
{
	
		for (int i = 0; i < m_pageDeque.size(); i++)
		{
			std::list<int>::iterator it;
			for (it = bookmarks.begin(); it != bookmarks.end(); ++it)
			{
				if (m_pageDeque.at(i).page_num == *it)
				{
					m_pageDeque.at(i).bookmark = true;
					break;
				}
				else 
				{
					m_pageDeque.at(i).bookmark = false;
				}
		}
	}
}

void PageBuilder::SaveImage(float percent, std::wstring path)
{
	m_reader->SaveImage(m_pageDeque.at(3).page_num, percent, path);
}
