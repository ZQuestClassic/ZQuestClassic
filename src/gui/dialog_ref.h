#ifndef ZC_GUI_DIALOGREF_H_
#define ZC_GUI_DIALOGREF_H_

#include <cstddef>
#include <cstdint>
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
	DIALOG& operator[](int32_t offset);
	const DIALOG& operator[](int32_t offset) const;
	operator bool() const
	{
		return owner;
	}
	
	void applyVisibility(bool visible, int32_t offs = 0);
	void applyDisabled(bool dis, int32_t offs = 0);
	int32_t message(int32_t msg, int32_t c);
	
private:
	DialogRunner* owner;
	size_t index;
	int32_t old_x;

	DialogRef(DialogRunner* owner, size_t index);

	friend class DialogRunner;
};

}

#endif
