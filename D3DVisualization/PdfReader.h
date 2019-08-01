#pragma once
#include <mupdf/fitz.h>
#include <string>
#include <Windows.h>


class PdfReader
{
public:
	PdfReader();
	~PdfReader();
	bool ReadPdfDocument(std::wstring filename, int page_num);
	int GetDocumentSize();
	int width();
	int height();
	unsigned char *GetUNORMPage();

	bool SearchText(int page_num, std::wstring searchtext);
	void SetCaseSensitiveSearch(bool aState);

	void SaveImage(int page_num, float percent, std::wstring path);

private:
	float aColTofCol(unsigned char color);
	


	char m_filename[MAX_PATH];
	fz_pixmap *m_pix;
	int m_page_count;
	float m_zoom;
	float m_rotate;
	fz_context *m_ctx;
	fz_document *m_doc;
	fz_matrix m_ctm;

	//search state
	int issearching;
	int searchdir;
	char search[512];
	int searchpage;
	fz_quad m_hit_bbox[512];
	int hit_count;
	int m_case_sensitive;
};
