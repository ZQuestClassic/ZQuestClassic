#ifndef EDITBOXNEW_H_
#define EDITBOXNEW_H_

#include <string>
#include <list>
#include <map>

using std::string;
using std::list;
using std::pair;

#ifdef _MSC_VER
#pragma warning(disable: 4355)
#endif

class EditboxModel;
class EditboxCursor;
struct DIALOG;
struct FONT;
struct BITMAP;

class Unicode
{
public:
	static const int32_t TABSIZE = 4;
	static int32_t indexToOffset(string &s, int32_t i);
	static void insertAtIndex(string &s, int32_t c, int32_t i);
	static void extractRange(string &s, string &dest, int32_t first, int32_t last);
	static void removeRange(string &s, int32_t first, int32_t last);
	static int32_t getCharAtIndex(string &s, int32_t i);
	static int32_t getCharWidth(int32_t c, FONT *f);
	static pair<int32_t, int32_t> munchWord(string &s, int32_t startoffset, FONT *f);
	static int32_t getCharWidth(const char *s, int32_t offset);
	static int32_t getCharAtOffset(const char *s, int32_t offset);
	static void textout_ex_nonstupid(BITMAP *bmp, FONT *f, string &s, int32_t x, int32_t y, int32_t fg, int32_t bg);
	static int32_t getIndexOfWidth(string &s, int32_t x, FONT *f);
	static int32_t getLength(string &s);
};

class TextSelection
{
public:
	TextSelection() : isselecting(false), start(0), end(0) {}
	void restartSelection(EditboxCursor &cursor);
	void adjustSelection(EditboxCursor &cursor);
	bool hasSelection();
	void ensureSelecting(EditboxCursor &cursor);
	bool isSelecting()
	{
		return isselecting;
	}
	void clearSelection();
	pair<int32_t, int32_t> getSelection();
	void doneSelection()
	{
		isselecting=false;
	}
private:
	bool isselecting;
	int32_t start;
	int32_t end;
};

struct LineData
{
	string line;
	int32_t numchars;
	bool newlineterminated;
	bool dirtyflag;
	BITMAP *strip;
};

class EditboxView
{
public:
	EditboxView(DIALOG *Host, FONT *TextFont) : textfont(TextFont), dbuf(NULL), host(Host), model(NULL) {}
	void initialize(EditboxModel *model);
	void lineDown();
	void lineUp();
	void pageUp();
	void pageDown();
	void lineHome(bool ctrl);
	void lineEnd(bool ctrl);
	void update();
	virtual void scrollUp() {}
	virtual void scrollDown() {}
	FONT *getFont()
	{
		return textfont;
	}
	virtual void draw();
	virtual void ensureCursorOnScreen() {}
	virtual ~EditboxView();
	virtual bool mouseClick(int32_t, int32_t)
	{
		return false;
	}
	virtual bool mouseDrag(int32_t, int32_t)
	{
		return false;
	}
	virtual bool mouseRelease(int32_t, int32_t)
	{
		return false;
	}
	DIALOG *getDialog()
	{
		return host;
	}
protected:
	virtual void enforceHardLimits() {}
	void invertRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
	virtual int32_t getAreaHeight()=0;
	FONT *textfont;
	BITMAP *dbuf;
	virtual void layoutPage()=0;
	virtual void init() {}
	DIALOG *host;
	EditboxModel *model;
};

struct CursorPos
{
	int32_t lineno;
	int32_t index;
	int32_t x;
	list<LineData>::iterator it;
};

class EditboxCursor
{
public:
	EditboxCursor(EditboxModel &model) : visible(true), host(model), index(0), preferredX(0) {}
	void insertChar(int32_t c);
	void insertString(string s);
	void updateCursor(int32_t new_index)
	{
		index = new_index;
	}
	void invertVisibility()
	{
		visible = !visible;
	}
	bool isVisible()
	{
		return visible;
	}
	int32_t getPosition()
	{
		return index;
	}
	void operator++(int32_t);
	void operator--(int32_t);
	int32_t getPreferredX()
	{
		return preferredX;
	}
	void setPreferredX();
	void insertNewline();
	void deleteChar();
private:
	bool visible;
	EditboxModel &host;
	int32_t index;
	int32_t preferredX;
	//NOT IMPLEMENTED: DO NOT USE
	EditboxCursor(EditboxCursor &);
	EditboxCursor &operator =(EditboxCursor &);
};

class EditboxModel
{
public:
	EditboxModel(string &Buffer, EditboxView *View, bool ReadOnly = false, char *hf = NULL)
		: helpfile(hf), lines(), buffer(Buffer), undobuf(""),
		undoindx(0), has_undo_point(false), view(View), readonly(ReadOnly),
		cursor(*this), clipboard(""), s()
	{}
	TextSelection &getSelection()
	{
		return s;
	}
	EditboxCursor &getCursor()
	{
		return cursor;
	}
	EditboxView *getView()
	{
		return view;
	}
	string &getBuffer()
	{
		return buffer;
	}
	~EditboxModel()
	{
		delete view;
	}
	list<LineData> &getLines()
	{
		return lines;
	}
	CursorPos findCursor();
	CursorPos findIndex(int32_t totalindex);
	void markAsDirty(list<LineData>::iterator line);
	void markAsDirty();
	bool isReadonly()
	{
		return readonly;
	}
	void makeLines(list<LineData> &target, string &source);
	void copy();
	void cut();
	void clear();
	void paste();
	void set_undo();
	void undo();
	void doHelp();
private:
	char *helpfile;
	list<LineData> lines;
	string &buffer;
	string undobuf;
	int32_t undoindx;
	bool has_undo_point;
	EditboxView *view;
	bool readonly;
	EditboxCursor cursor;
	string clipboard;
	TextSelection s;
	//NOT IMPLEMENTED; DO NOT USE
	EditboxModel &operator=(EditboxModel &);
};

struct CharPos
{
	list<LineData>::iterator it;
	int32_t lineIndex;
	int32_t totalIndex;
};

class BasicEditboxView : public EditboxView
{
public:
	BasicEditboxView(DIALOG *Host, FONT *TextFont, int32_t FGColor, int32_t BGColor, int32_t Highlight_Style) : EditboxView(Host, TextFont),
		view_x(0), view_y(0), fgcolor(FGColor), bgcolor(BGColor), hstyle(Highlight_Style) {}
	~BasicEditboxView();
	void ensureCursorOnScreen();
	void scrollUp();
	void scrollDown();
	void scrollLeft();
	void scrollRight();
	void draw();
	bool mouseClick(int32_t x, int32_t y);
	bool mouseDrag(int32_t x, int32_t y);
	bool mouseRelease(int32_t x, int32_t y);
	static const int32_t HSTYLE_EOLINE = 0;
	static const int32_t HSTYLE_EOTEXT = 1;
	int32_t getForeground()
	{
		return fgcolor;
	}
	int32_t getBackground()
	{
		return bgcolor;
	}
	void setForeground(int color)
	{
		fgcolor = color;
	}
	void setBackground(int color)
	{
		bgcolor = color;
	}
protected:
	void createStripBitmap(list<LineData>::iterator it, int32_t width);
	virtual void drawExtraComponents()=0;
	void init();
	int32_t getAreaHeight()
	{
		return area_height;
	}
	void enforceHardLimits();
	int32_t area_xstart;
	int32_t area_ystart;
	int32_t area_width;
	int32_t area_height;
	int32_t view_width;
	int32_t view_x;
	int32_t view_y;
	
	CharPos findCharacter(int32_t x, int32_t y);
	int32_t fgcolor;
	int32_t bgcolor;
	int32_t hstyle;
};

class EditboxVScrollView : public BasicEditboxView
{
public:
	EditboxVScrollView(DIALOG *Host, FONT *TextFont, int32_t FGColor, int32_t BGColor, int32_t Highlight_Style=HSTYLE_EOLINE) : BasicEditboxView(Host, TextFont, FGColor, BGColor, Highlight_Style), sbarpattern(NULL) {}
	
	~EditboxVScrollView();
	bool mouseClick(int32_t x, int32_t y);
	bool mouseDrag(int32_t x, int32_t y);
	bool mouseRelease(int32_t x, int32_t y);
protected:
	virtual bool mouseDragOther(int32_t x, int32_t y)
	{
		x=x;
		y=y; /*these are here to bypass compiler warnings about unused arguments*/ return false;
	}
	virtual bool mouseClickOther(int32_t x, int32_t y)
	{
		x=x;
		y=y; /*these are here to bypass compiler warnings about unused arguments*/ return false;
	}
	void drawExtraComponents();
	void init();
	int32_t bottomarrow_y;
	BITMAP *sbarpattern;
private:
	int32_t toparrow_x;
	int32_t toparrow_y;
	int32_t toparrow_state;
	int32_t bottomarrow_x;
	int32_t bottomarrow_state;
	int32_t baroff;
	int32_t barlen;
	int32_t barstate;
	int32_t barstarty;
};

class EditboxWordWrapView : public EditboxVScrollView
{
public:
	EditboxWordWrapView(DIALOG *Host, FONT *TextFont, int32_t FGColor, int32_t BGColor, int32_t Highlight_Style=HSTYLE_EOLINE) : EditboxVScrollView(Host, TextFont, FGColor, BGColor, Highlight_Style) {}
protected:
	void layoutPage();
};

class EditboxNoWrapView : public EditboxVScrollView
{
public:
	EditboxNoWrapView(DIALOG *Host, FONT *TextFont, int32_t FGColor, int32_t BGColor, int32_t Highlight_Style=HSTYLE_EOLINE) : EditboxVScrollView(Host, TextFont, FGColor, BGColor, Highlight_Style) {}
	void init();
protected:
	void layoutPage();
	void drawExtraComponents();
	bool mouseDragOther(int32_t x, int32_t y);
	bool mouseRelease(int32_t x, int32_t y);
	bool mouseClickOther(int32_t x, int32_t y);
	int32_t leftarrow_y;
	int32_t rightarrow_y;
private:
	int32_t leftarrow_x;
	int32_t leftarrow_state;
	int32_t rightarrow_x;
	int32_t rightarrow_state;
	int32_t hbaroff;
	int32_t hbarlen;
	int32_t hbarstate;
	int32_t hbarstartx;
};

class EditboxScriptView : public EditboxNoWrapView
{
public:
	EditboxScriptView(DIALOG *Host, FONT *TextFont, int32_t FGColor, int32_t BGColor, int32_t Highlight_Style=HSTYLE_EOLINE) : EditboxNoWrapView(Host, TextFont, FGColor, BGColor, Highlight_Style), linetext(NULL) {}
	void init();
	~EditboxScriptView();
protected:
	void drawExtraComponents();
private:
	BITMAP *linetext;
};

#endif

