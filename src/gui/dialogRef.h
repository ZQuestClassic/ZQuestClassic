#ifndef ZC_GUI_DIALOGREF_H
#define ZC_GUI_DIALOGREF_H

#include <cstddef>
struct DIALOG;

namespace gui
{

class DialogRunner;
/* References an item in the DialogRunner's DIALOG array, which isn't safe
 * to do directly because it might be reallocated as the vector grows.
 * operator[] can also be used to access other DIALOGs in the array.
 */
class DialogRef
{
public:
	DialogRef();
	DIALOG* operator->();
	const DIALOG* operator->() const;
	DIALOG& operator[](int offset);
	const DIALOG& operator[](int offset) const;
	operator bool() const;

private:
	DialogRunner* owner;
	size_t index;

	DialogRef(DialogRunner* owner, size_t index);

	friend class DialogRunner;
};

}

#endif
