#ifndef EDITBOXNEW_H
#define EDITBOXNEW_H

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
	static const int TABSIZE = 4;
	static int indexToOffset(string &s, int i);
	static void insertAtIndex(string &s, int c, int i);
	static void extractRange(string &s, string &dest, int first, int last);
	static void removeRange(string &s, int first, int last);
	static int getCharAtIndex(string &s, int i);
	static int getCharWidth(int c, FONT *f);
	static pair<int, int> munchWord(string &s, int startoffset, FONT *f);
	static int getCharWidth(const char *s, int offset);
	static int getCharAtOffset(const char *s, int offset);
	static void textout_ex_nonstupid(BITMAP *bmp, FONT *f, string &s, int x, int y, int fg, int bg);
	static int getIndexOfWidth(string &s, int x, FONT *f);
	static int getLength(string &s);
};

class TextSelection
{
public:
	TextSelection() : isselecting(false), start(0), end(0) {}
	void restartSelection(EditboxCursor &cursor);
	void adjustSelection(EditboxCursor &cursor);
	bool hasSelection();
	void ensureSelecting(EditboxCursor &cursor);
	bool isSelecting() {return isselecting;}
	void clearSelection();
	pair<int, int> getSelection();
	void doneSelection() {isselecting=false;}
private:
	bool isselecting;
	int start;
	int end;
};

struct LineData
{
	string line;
	int numchars;
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
	void lineHome();
	void lineEnd();
	void update();
	virtual void scrollUp() {}
	virtual void scrollDown() {}
	FONT *getFont() {return textfont;}
	virtual void draw();
	virtual void ensureCursorOnScreen() {}
	virtual ~EditboxView();
	virtual bool mouseClick(int, int) {return false;}
	virtual bool mouseDrag(int, int) {return false;}
	virtual bool mouseRelease(int, int) {return false;}
	DIALOG *getDialog() {return host;}
protected:
	virtual void enforceHardLimits() {}
	void invertRectangle(int x1, int y1, int x2, int y2);
	virtual int getAreaHeight()=0;
	FONT *textfont;
	BITMAP *dbuf;
	virtual void layoutPage()=0;
	virtual void init() {}
	DIALOG *host;
	EditboxModel *model;
};

struct CursorPos
{
	int lineno;
	int index;
	int x;
	list<LineData>::iterator it;
};

class EditboxCursor
{
public:
	EditboxCursor(EditboxModel &model) : visible(true), host(model), index(0), preferredX(0) {}
	void insertChar(int c);
	void insertString(string s);
	void updateCursor(int new_index) {index = new_index;}
	void invertVisibility() {visible = !visible;}
	bool isVisible() {return visible;}
	int getPosition() {return index;}
	void operator++(int);
	void operator--(int);
	int getPreferredX() {return preferredX;}
	void setPreferredX();
	void insertNewline();
	void deleteChar();
private:
	bool visible;
	EditboxModel &host;
	int index;
	int preferredX;
	//NOT IMPLEMENTED: DO NOT USE
	EditboxCursor(EditboxCursor &);
	EditboxCursor &operator =(EditboxCursor &);
};

class EditboxModel
{
public:
	EditboxModel(string &Buffer, EditboxView *View, bool ReadOnly = false, char *hf = NULL) : helpfile(hf), lines(), buffer(Buffer), view(View), readonly(ReadOnly), cursor(*this), clipboard(""), s() {}
	TextSelection &getSelection() {return s;}
	EditboxCursor &getCursor() {return cursor;}
	EditboxView *getView() {return view;}
	string &getBuffer() {return buffer;}
	~EditboxModel() {delete view;}
	list<LineData> &getLines() {return lines;}
	CursorPos findCursor();
	CursorPos findIndex(int totalindex);
	void markAsDirty(list<LineData>::iterator line);
	bool isReadonly() {return readonly;}
	void makeLines(list<LineData> &target, string &source);
	void copy();
	void cut();
	void clear();
	void paste();
	void doHelp();
private:
	char *helpfile;
	list<LineData> lines;
	string &buffer;
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
	int lineIndex;
	int totalIndex;
};

class BasicEditboxView : public EditboxView
{
public:
	BasicEditboxView(DIALOG *Host, FONT *TextFont, int FGColor, int BGColor, int Highlight_Style) : EditboxView(Host, TextFont), 
		view_x(0), view_y(0), fgcolor(FGColor), bgcolor(BGColor), hstyle(Highlight_Style) {}
	~BasicEditboxView();
	void ensureCursorOnScreen();
	void scrollUp();
	void scrollDown();
	void scrollLeft();
	void scrollRight();
	void draw();
	bool mouseClick(int x, int y);
	bool mouseDrag(int x, int y);
	bool mouseRelease(int x, int y);
	static const int HSTYLE_EOLINE = 0;
	static const int HSTYLE_EOTEXT = 1; 
	int getForeground() {return fgcolor;}
	int getBackground() {return bgcolor;}
protected:
	void createStripBitmap(list<LineData>::iterator it, int width);
	virtual void drawExtraComponents()=0;
	void init();
	int getAreaHeight() {return area_height;}
	void enforceHardLimits();
	int area_xstart;
	int area_ystart;
	int area_width;
	int area_height;
	int view_width;
	int view_x;
	int view_y;
	
	CharPos findCharacter(int x, int y);
	int fgcolor;
	int bgcolor;
	int hstyle;
};

class EditboxVScrollView : public BasicEditboxView
{
public:
	EditboxVScrollView(DIALOG *Host, FONT *TextFont, int FGColor, int BGColor, int Highlight_Style=HSTYLE_EOLINE) : BasicEditboxView(Host, TextFont, FGColor, BGColor, Highlight_Style), sbarpattern(NULL) {}
	
	~EditboxVScrollView();
	bool mouseClick(int x, int y);
	bool mouseDrag(int x, int y);
	bool mouseRelease(int x, int y);
protected:
	virtual bool mouseDragOther(int x, int y) {x=x; y=y; /*these are here to bypass compiler warnings about unused arguments*/ return false;}
	virtual bool mouseClickOther(int x, int y) {x=x; y=y; /*these are here to bypass compiler warnings about unused arguments*/ return false;}
	void drawExtraComponents();
	void init();
	int bottomarrow_y;
	BITMAP *sbarpattern;
private:
	int toparrow_x;
	int toparrow_y;
	int toparrow_state;
	int bottomarrow_x;
	int bottomarrow_state;
	int baroff;
	int barlen;
	int barstate;
	int barstarty;
	
};

class EditboxWordWrapView : public EditboxVScrollView
{
public:
	EditboxWordWrapView(DIALOG *Host, FONT *TextFont, int FGColor, int BGColor, int Highlight_Style=HSTYLE_EOLINE) : EditboxVScrollView(Host, TextFont, FGColor, BGColor, Highlight_Style) {}
protected:
	void layoutPage();
};

class EditboxNoWrapView : public EditboxVScrollView
{
public:
	EditboxNoWrapView(DIALOG *Host, FONT *TextFont, int FGColor, int BGColor, int Highlight_Style=HSTYLE_EOLINE) : EditboxVScrollView(Host, TextFont, FGColor, BGColor, Highlight_Style) {}
	void init();
protected:
	void layoutPage();
	void drawExtraComponents();
	bool mouseDragOther(int x, int y);
	bool mouseRelease(int x, int y);
	bool mouseClickOther(int x, int y);
	int leftarrow_y;
	int rightarrow_y;
private:
	int leftarrow_x;
	int leftarrow_state;
	int rightarrow_x;
	int rightarrow_state;
	int hbaroff;
	int hbarlen;
	int hbarstate;
	int hbarstartx;
};

class EditboxScriptView : public EditboxNoWrapView
{
public:
	EditboxScriptView(DIALOG *Host, FONT *TextFont, int FGColor, int BGColor, int Highlight_Style=HSTYLE_EOLINE) : EditboxNoWrapView(Host, TextFont, FGColor, BGColor, Highlight_Style), linetext(NULL) {}
	void init();
	~EditboxScriptView();
protected:
	void drawExtraComponents();
private:
	BITMAP *linetext;
};

#endif
 
