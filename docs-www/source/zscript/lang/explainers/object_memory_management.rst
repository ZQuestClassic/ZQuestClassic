.. _gc:

Object Memory Management
========================

.. |GlobalObject| replace:: :ref:`GlobalObject<globals_fun_globalobject>`
.. |OwnObject| replace:: :ref:`OwnObject<globals_fun_ownobject>`
.. |OwnArray| replace:: :ref:`OwnArray<globals_fun_ownarray>`

.. versionadded:: 3.0
	\ 

	Pre-3.0, objects had to be manually deleted, but with ZC 3.0 there is now a garbage
	collector to automate freeing objects for scripters.

	TL;DR for experienced ZScript users: you don't need to use `delete` or `Free()` anymore, and
	you probably never need to use `->Own()` either. Read on for specifics.

What's managed
--------------

The following types are objects that are tracked by the garbage collector:

.. hlist::
	:columns: 4

	- :ref:`arrays / strings<arrays>`
	- :ref:`any custom user class<classes>`
	- :ref:`bitmap<classes_bitmap>`
	- :ref:`directory<classes_directory>`
	- :ref:`file<classes_file>`
	- :ref:`paldata<classes_paldata>`
	- :ref:`randgen<classes_randgen>`
	- :ref:`stack<classes_stack>`
	- :ref:`websocket<classes_websocket>`

Note: there are other object types, but they are never explicitly created or deleted by scripts.

How it works
------------

The ZScript garbage collector has two ways for knowing when to delete objects:

1. Reference counting - variables and arrays containing an object count as references, and
   as long as there is one reference to an object it will not be deleted
2. Reachability - objects that are not reachable from a global object, local/global variable,
   or local/global array are considered unreachable, and will be deleted. This is necessary
   to detect objects that may always have a reference count greater than zero (for example:
   two objects holding a reference to each other, but otherwise no longer in use). The process
   of detecting unreachable objects is a "full garbage collection", and only runs occasionally.

Variables only retain a reference to an object if the variable is typed as an object.
Storing an object pointer in an `int` or `untyped` variable does *not* count as a
reference, so the object may be deleted while that variable still points at it:

.. zscript::
	:style: body

	bitmap b = Game->CreateBitmap(64, 64); // 'b' retains a reference
	untyped u = Game->CreateBitmap(64, 64); // no reference! may be deleted at the next Waitframe
	int i = <int>(Game->CreateBitmap(64, 64)); // no reference either

Containers are different. When a value is stored into an `untyped[]` array or a
`stack<untyped>`, the compiler records whether that value is an object, so these
containers *do* retain the objects put into them. An `int[]` array never does:

.. zscript::
	:style: body

	untyped arr[2];
	arr[0] = Game->CreateBitmap(64, 64); // retained by the array

	stack<untyped> st = new stack<untyped>();
	st->PushBack(Game->CreateBitmap(64, 64)); // retained by the stack

	int nums[2];
	nums[0] = <int>(Game->CreateBitmap(64, 64)); // NOT retained - 'int[]' never holds objects

If you must store an object in a plain `int` or `untyped` variable, call |OwnObject| (for custom
objects) or ``->Own`` (for builtin objects like bitmap) on it so its reference count is always
greater than zero (as long as that script is alive). For a custom object, you can instead
globalize it via |GlobalObject|.

.. zscript::
	:style: body

	bitmap b = Game->CreateBitmap(64, 64);
	b->Own(); // the running script now holds a reference
	untyped u = b; // safe: the object outlives this variable

.. important::

	|OwnObject| and |GlobalObject| only accept :ref:`custom class<classes>` objects. Both take
	an `untyped` parameter, so the compiler cannot catch a misuse - passing any other kind of
	object compiles fine, then logs a script error and does nothing at runtime. Each kind of
	object has its own way to take ownership:

	- :ref:`custom class<classes>` instances: |OwnObject|
	- builtin objects (`bitmap`, `directory`, `file`, `paldata`, `randgen`, `stack`, `websocket`): `obj->Own()`
	- :ref:`arrays and strings<arrays>`: |OwnArray|

	Likewise, only custom class objects can be globalized. To keep any other object alive
	indefinitely, store it in a global variable (or inside a global object) - that also makes
	it persist to the save file.

Objects made global with |GlobalObject| are never deleted by the garbage collector.

To tie an object to at least the lifetime of the currently running script, take ownership of
it (|OwnObject|, `obj->Own()`, or |OwnArray|, as listed above).
This associates a reference to the object with the current script, so when that script ends
if there are no other references to the object it will be destroyed. Only one script may
own a particular object - subsequent calls transfer which script holds the reference.
You shouldn't need this functionality for most usages. One example of it being necessary
is if the only place you store an object is a plain `untyped` variable, as shown above.

For simple objects with no cyclical references, they are destroyed just after their last
reference is removed (as local variable going out of scope, or being overwritten, etc).

New objects have an implicit reference added to an "autorelease pool" (a construct borrowed
from Objective-C). This reference is removed the next time the script yields (such as by
calling Waitframe), or the first time it is stored somewhere (such as being assigned to an object
variable, or being inserted into array). If the object has not been stored somewhere by then, or
made global, it is deleted when the script yields.

For objects with cyclical references (or self-references), their deletion is deferred until
a full garbage collection run. When an object is found to be unreachable by
any global or local variable, and it is not in the autorelease pool, it will be deleted. This
process is much more expensive than reference counting, so it doesn't run often, and when
exactly it runs is subject to change.

For a custom class object, script ownership can be revoked by calling |GlobalObject|. You can
delete a global object by making it no longer global (call |OwnObject| on it) - once all other
references are unassigned, the garbage collector will eventually delete it.

When a script terminates, objects it owns will lose a reference count, calling their
destructors immediately if their reference count is now zero.

On save, an object persists to the save file (including all its variables and arrays) if it has
been globalized via |GlobalObject|, or if it is reachable from a global variable or array (note:
int arrays do not retain objects, but untyped arrays do).

.. versionchanged:: 3.0
	Prior to this version, only globalized objects persist to the save file.

.. caution::
	References to internal arrays (such as :ref:`Screen->D[]<globals_screen_var_d>`) never persist to save files. They will
	be replaced with null.

.. caution::
	You should not expect destructors to run at any specific time, or even at all. You should only
	use destructors for debugging purposes, or for deleting/disowning other objects. Exactly when
	the garbage collector runs or an object destructor is called is an implementation detail that
	may change. Do NOT implement critical game functionality in destructors - doing so will result
	in unpredictable behavior, and may break your quest in future versions of ZC.
