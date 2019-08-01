#include "PdfReader.h"
#define C_BTOF 0.00392156862745 


PdfReader::PdfReader() :
	 m_zoom(100)
	,m_rotate(0)
	,m_page_count(0)
	,m_case_sensitive(false)
{
}

PdfReader::~PdfReader()
{
	fz_drop_pixmap(m_ctx, m_pix);
	fz_drop_document(m_ctx, m_doc);
	fz_drop_context(m_ctx);
}


int PdfReader::GetDocumentSize()
{
	return m_page_count;
}

int PdfReader::width()
{
	return m_pix->w;
}

int PdfReader::height()
{
	return m_pix->h;
}

bool PdfReader::SearchText(int page_num, std::wstring searchtext)
{
	wchar_t filename[MAX_PATH];
	int code = MultiByteToWideChar(CP_UTF8, 0, m_filename, -1, filename, MAX_PATH);
	if (code == 0)
		MessageBox(nullptr, L"Не удалось конвертировать в wchar*", L"Error", MB_OK);
	ReadPdfDocument(filename, page_num);

	char searchtxt[MAX_PATH];

	code = WideCharToMultiByte(CP_UTF8, 0, searchtext.c_str(), -1, searchtxt, MAX_PATH, NULL, NULL);
	if (code == 0)
		MessageBox(nullptr, L"Не удалось конвертировать в utf-8", L"Error", MB_OK);


	int firstpage, page;
	firstpage = 0;
	page = m_page_count - 1;

	//fz_ne

	//fz_new_stext_page(m_ctx,)
	fz_stext_page *page_text;
	fz_cookie cookie = { 0 };

	fz_try(m_ctx)
	{
		fz_page *page_for_s = fz_load_page(m_ctx, m_doc, page_num);
		fz_rect page_bbox = fz_bound_page(m_ctx, page_for_s);
		page_text = fz_new_stext_page(m_ctx, page_bbox);
		fz_device *tdev = fz_new_stext_device(m_ctx, page_text, NULL);
		fz_display_list *d_list = fz_new_display_list(m_ctx, fz_infinite_rect);
		fz_device *mdev = fz_new_list_device(m_ctx, d_list);
		fz_enable_device_hints(m_ctx, mdev, FZ_NO_CACHE);
		fz_run_page_contents(m_ctx, page_for_s, mdev, fz_identity, &cookie);
		fz_close_device(m_ctx, mdev);
		fz_drop_device(m_ctx, mdev);

		fz_run_display_list(m_ctx, d_list, tdev, m_ctm, fz_infinite_rect, &cookie);
		fz_close_device(m_ctx, tdev);
	}
	fz_catch(m_ctx)
	{
		MessageBox(nullptr, L"Не удалось открыть документ", L"Error", MB_OK);
		fz_drop_context(m_ctx);
		return EXIT_FAILURE;
	}

	//do
	//{
		hit_count = fz_search_stext_page_case_sensitive(m_ctx, page_text, searchtxt, m_hit_bbox, sizeof(m_hit_bbox) / sizeof(m_hit_bbox[0]), (int)m_case_sensitive);
		//hit_count = fz_search_stext_page(m_ctx, page_text, searchtxt, m_hit_bbox, sizeof(m_hit_bbox) / sizeof(m_hit_bbox[0]));
	//}
	for(int i = 0; i < hit_count; i++)
		fz_invert_pixmap_rect(m_ctx, m_pix, fz_round_rect(fz_rect_from_quad(m_hit_bbox[i])));

		
		//pixmapToDocForRender();
	//fz_bookmark dsad = fz_make_bookmark(m_ctx, m_doc, 2);

	if (hit_count > 0)
		return true;
	else
		return false;
}


bool PdfReader::ReadPdfDocument(std::wstring filename, int page_num)
{
	int code = WideCharToMultiByte(CP_UTF8, 0, filename.c_str(), -1, m_filename, MAX_PATH, NULL, NULL);
	if (code == 0)
	{
		MessageBox(nullptr, L"Не удалось конвертировать в utf-8", L"Error", MB_OK);
		return EXIT_FAILURE;
	}

	m_ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!m_ctx)
	{
		MessageBox(nullptr, L"Не удалось создать контекст mupdf", L"Error", MB_OK);
		return EXIT_FAILURE;
	}

	fz_try(m_ctx)
		fz_register_document_handlers(m_ctx);
	fz_catch(m_ctx)
	{
		MessageBox(nullptr, L"Не удалось зарегестрировать хендл документа", L"Error", MB_OK);
		fz_drop_context(m_ctx);
		return EXIT_FAILURE;
	}

	/* Open the document. */
	fz_try(m_ctx)
		m_doc = fz_open_document(m_ctx, m_filename);
	fz_catch(m_ctx)
	{
		MessageBox(nullptr, L"Не удалось открыть документ", L"Error", MB_OK);
		fz_drop_context(m_ctx);
		return EXIT_FAILURE;
	}
	/* Count the number of pages. */
	fz_try(m_ctx)
		m_page_count = fz_count_pages(m_ctx, m_doc);
	fz_catch(m_ctx)
	{
		MessageBox(nullptr, L"Не удалось посчитать количество страниц", L"Error", MB_OK);
		fz_drop_document(m_ctx, m_doc);
		fz_drop_context(m_ctx);
		return EXIT_FAILURE;
	}

	if (page_num < 0 || page_num >= m_page_count)
	{
		MessageBox(nullptr, L"Не допустимый номер страницы", L"Error", MB_OK);
		fz_drop_document(m_ctx, m_doc);
		fz_drop_context(m_ctx);
		return EXIT_FAILURE;
	}

	/* Compute a transformation matrix for the zoom and rotation desired. */
	/* The default resolution without scaling is 72 dpi. */
	m_ctm = fz_scale(m_zoom / 100, m_zoom / 100);
	m_ctm = fz_pre_rotate(m_ctm, m_rotate);

	/* Render page to an RGB pixmap. */
	fz_try(m_ctx)
	{
		fz_page *page;
		m_pix = NULL;
		page = fz_load_page(m_ctx, m_doc, page_num);
		m_pix = fz_new_pixmap_from_page(m_ctx, page, m_ctm, fz_device_rgb(m_ctx), 0);
		//m_pix = fz_new_pixmap_from_page_number(m_ctx, m_doc, page_num, m_ctm, fz_device_rgb(m_ctx), 1);
	}
	fz_catch(m_ctx)
	{
		MessageBox(nullptr, L"Не удалось запомнить страницу", L"Error", MB_OK);
		fz_drop_document(m_ctx, m_doc);
		fz_drop_context(m_ctx);
		return EXIT_FAILURE;
	}

	return true;
}

float PdfReader::aColTofCol(unsigned char color)
{
	int value = color;
	return (float)value * C_BTOF;
}

unsigned char *PdfReader::GetUNORMPage()
{
	int width = 0;
	int height = 0;
	int c = 0;

	width = m_pix->w;
	height = m_pix->h;

	unsigned char *page = new unsigned char[width * height * 4];

	for (int y = 0; y < height; ++y)
	{
		unsigned char *p = &m_pix->samples[y * m_pix->stride];
		for (int x = 0; x < width; ++x)
		{
			page[c] = p[0];
			page[c + 1] = p[1];
			page[c + 2] = p[2];
			page[c + 3] = 'я';
			c += 4;
			p += m_pix->n;
		}
	}

	return page;
}

void PdfReader::SetCaseSensitiveSearch(bool aState)
{
	m_case_sensitive = aState;
}

void PdfReader::SaveImage(int page_num, float percent, std::wstring path)
{
	char cpath[MAX_PATH];

	int code = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, cpath, MAX_PATH, NULL, NULL);
	if (code == 0)
		MessageBox(nullptr, L"Не удалось конвертировать в utf-8", L"Error", MB_OK);

	m_ctm = fz_scale(percent / 100.0f, percent / 100.0f);

	fz_try(m_ctx)
	{
		fz_page *page;
		m_pix = NULL;
		page = fz_load_page(m_ctx, m_doc, page_num);
		m_pix = fz_new_pixmap_from_page(m_ctx, page, m_ctm, fz_device_rgb(m_ctx), 0);
		fz_save_pixmap_as_png(m_ctx, m_pix, cpath);
		//fz_save_pixmap_as_png(m_ctx, m_pix, "d:\\pixmap.png");
	}
	fz_catch(m_ctx)
	{
		MessageBox(nullptr, L"Не удалось записать .png", L"Error", MB_OK);
		fz_drop_document(m_ctx, m_doc);
		fz_drop_context(m_ctx);
	}
}
