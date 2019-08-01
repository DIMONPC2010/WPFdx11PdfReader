#pragma once
#include <memory>
#include <string>
#include "PdfReader.h"
#include <deque>
#include <list>
#define PAGE_NUM 7

struct Page
{
	Page() : bookmark(false) {};
	unsigned char *RGB;
	int width;
	int height;
	int page_num;
	bool bookmark;
};

class PageBuilder
{
private:
	typedef std::shared_ptr<PdfReader> PdfReader_t;

public:
	PageBuilder();
	PageBuilder(std::wstring filename);
	void GetStartPages();
	unsigned char *GetPage(int page_num);
	bool SearchOnPageForward(int page_num, std::wstring searchtext);
	bool SearchOnPageBackward(int page_num, std::wstring searchtext);
	void SetCaseSensitiveSearch(bool aState);
	int size();
	int width(int page_num);
	int height(int page_num);
	int NowWidth();
	int NowHeight();
	void GetNext();
	void GetPrevious();
	int NowView();
	bool Bookmark(int page_num);
	void SetBookmark(bool aState);
	void CheckBookmarks(std::list<int> bookmarks);

	void SaveImage(float percent, std::wstring path);

private:
	std::wstring m_filename;
	std::deque<Page> m_pageDeque;
	int m_size;
protected:
	PdfReader_t m_reader;

};