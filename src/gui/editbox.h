#ifndef EDITBOX_H_
#define EDITBOX_H_

#include "base/zc_alleg.h"
#include "base/headers.h"
#include <functional>
#include <string>

enum {eb_wrap_none, eb_wrap_char, eb_wrap_word};
enum {eb_crlf_n, eb_crlf_nr, eb_crlf_r, eb_crlf_rn, eb_crlf_any};
enum {eb_scrollbar_optional, eb_scrollbar_on, eb_scrollbar_off};
enum {eb_handle_vscroll, eb_handle_hscroll};

int32_t d_editbox_proc(int32_t msg, DIALOG *d, int32_t c);

typedef struct editbox_data
{
	std::string text;
	int32_t showcursor;
	int32_t lines;
	int32_t currtextline;
	int32_t list_width;
	int32_t currxpos;
	int32_t fakexpos;
	int32_t xofs;
	int32_t yofs;
	int32_t maxchars;
	int32_t maxlines;
	int32_t wrapping;
	int32_t insertmode;
	int32_t currchar;
	int32_t tabdisplaystyle;
	int32_t crlfdisplaystyle;
	int32_t newcrlftype;
	int32_t vsbarstyle;
	int32_t hsbarstyle;
	FONT *font;
	//char *clipboard;
	//int32_t clipboardsize;
	std::string clipboard;
	int32_t defaulttabsize;
	int32_t tabunits;
	int32_t customtabs;
	int32_t *customtabpos;
	int32_t numchars;
	int32_t selstart;
	int32_t selend;
	int32_t selfg;
	int32_t selbg;
	int32_t postpaste_dontmove;
} editbox_data;

bool do_box_edit(string& str, string const& title, bool wrap, bool rdonly, bool trimstr = false, char const* helpfile = nullptr);
bool do_box_edit(FILE* f, string const& title, bool wrap, char const* helpfile = nullptr);
bool do_box_edit(DIALOG* dlg, std::function<bool(int)> proc, string& str, string const& title, bool wrap, bool rdonly, bool trimstr = false, char const* helpfile = nullptr);
bool do_box_edit(DIALOG* dlg, std::function<bool(int)> proc, FILE* f, string const& title, bool wrap, char const* helpfile = nullptr);

void do_box_setup(DIALOG* dlg);

#endif
