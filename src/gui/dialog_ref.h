#ifndef ZC_GUI_DIALOGREF_H
#define ZC_GUI_DIALOGREF_H

#include <cstddef>
struct DIALOG;

namespace GUI
{

class DialogRunner;
/* References an item in the DialogRunner's DIALOG array, which isn't safe
 * to do directly because it might be reallocated as the vector grows.
 * operator[] can also be used to access other DIALOGs in the array.
 */
class DialogRef
{
public:
	// All of these functions could reasonably be inlined, but DialogRunner
	// needs DialogRef defined ahead of it.
	// They could both go in the same file...
	DialogRef();
	DIALOG* operator->();
	const DIALOG* operator->() const;
	DIALOG& operator*();
	DIALOG& operator[](int offset);
	const DIALOG& operator[](int offset) const;
	operator bool() const
	{
		return owner;
	}
	
	void applyVisibility(bool visible);
	void message(int msg, int c);
	
private:
	DialogRunner* owner;
	size_t index;
	int old_x;

	DialogRef(DialogRunner* owner, size_t index);

	friend class DialogRunner;
};

}

#endif
