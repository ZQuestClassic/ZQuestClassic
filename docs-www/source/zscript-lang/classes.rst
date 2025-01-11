Classes
=========

.. |GlobalObject| replace:: :ref:`GlobalObject<globals_fun_globalobject>`
.. |OwnObject| replace:: :ref:`OwnObject<classes_sprite_fun_ownobject>`

.. _zslang_classes:

.. _classes:

What is a Class
---------------

The concept of a ``class`` is taken mostly from C++ here, although notably simplified down.
By declaring a ``class``, you declare a template of data to be used repeatedly in creating
``objects``, along with some functions associated with that data.

While the order of the parts within a class mostly doesn't matter to the compiler, you will
generally see classes organized in the same general order\ [#decl]_\ -

1. Data: The variables that make up the class. Can contain arrays, too.
	- While technically not 'required', a class without data has little purpose.
	- Declared via normal variable declarations. Cannot have initializers.
2. Constructor(s): A function or functions to create the class.
	- Not required- leaving it out is the same as declaring an empty constructor function.
	- The constructor is declared as a function with the exact same name as the class and no return type.
	- The parameters may be whatever you like.
3. Destructor: A function to destroy the class.
	- Not required- leaving it out is *better for the compiler* than declaring an empty destructor function.
	- A destructor is declared as a function named ``~`` followed by the exact same name as the class and no return type.
	- The destructor takes no parameters.
	- The destructor is automatically called when the object is destroyed by the engine.
		.. versionchanged:: 3.0
			The destructor can no longer manually be triggered with ``delete``.
			This is no longer necessary due to the :ref:`refcounting + garbage collector<gc>`.
4. Member Functions: Any other functions you like.
	- All class functions (without the ``static`` modifier) have a local variable named ``this``, which is a pointer to the object itself.

.. note::
	The order the data is declared in is relevant to existing save files, for any objects saved to
	the save file. As such, changing the order can break existing save files
	(in the same way that declaring *any* new global variable can)

Allocating Objects
------------------

.. _classobj_alloc:

To create an instance of an object, you call it's constructor function using the ``new`` keyword\ [#new]_\ .
A pointer to the object, whose type is *the class itself*, will be returned.

.. admonition:: Limits
	:class: note

	A maximum of ``214747`` objects can exist at a time, from all (non-internal) classes combined.
	If this many objects exist at once, the ``new`` call will fail, returning ``NULL``.

Object Cleanup
--------------

.. toctree::
	:hidden:
	:maxdepth: 1

	/zscript-lang/explainers/object_memory_management

:ref:`Full Explainer<gc>`

The program keeps track of all the references to each object, and automatically deletes any object
when the last reference to it is lost (calling the destructor, if defined).

There is additionally a garbage collector, which is capable of cleaning up cyclical references
(the situation in which the only reference left to two different objects, is inside the other,
thus they 'keep each other alive' for the reference counting)

.. tip::
	|OwnObject| grants a single "reference" to the object to
	whichever entity the function is giving ownership to. If that entity dies, it causes the
	ownership reference to be 'lost'.

.. tip::
	|GlobalObject| marks an object as "global". Objects that
	are "global" will be saved to the save file, and cannot be destroyed.

	Calling |OwnObject| marks the object as no longer "global".

Static Functions
----------------

Declaring a function inside a class with the ``static`` modifier makes the function a static part of
the class, rather than a part of each object. Static functions have a few differences from normal
member functions:

- No ``this`` pointer (meaning no access to any data members from the class)
- Called similarly to :ref:`functions inside namespaces<namespaces>`, except that a ``.`` is used after the class name, rather than the scope-resolution operator ``::``

Examples
--------

.. [#decl]
	*Declaration of a class*

	.. zscript::
		class Rectangle
		{
			int x, y; // The position of the rectangle
			int w, h; // The width/height of the rectangle
			int color; // The color of the rectangle
			int opacity; // OP_OPAQUE or OP_TRANS
			bool fill;

			// The 'Constructor' of the class - used to create new instances
			Rect(int X, int Y, int W, int H, int COLOR = 0x01,
				bool FILL = true, int OPACITY = OP_OPAQUE)
			{
				x = X;
				y = Y;
				w = W;
				h = H;
				color = COLOR;
				fill = FILL;
				opacity = OPACITY;
			}
			// A member function of the class, used alongside the data
			void draw(int layer)
			{
				Screen->Rectangle(layer, x, y, x+w-1, y+h-1,
					color, 1, 0, 0, 0, fill, opacity);
			}
		};

.. [#new]
	*Example script making use of the class from above*
	
	.. zscript::
		ffc script screenBorder
		{
			void run(int color, int thickness)
			{
				// Declare 4 'Rectangle' objects that form a border around the screen
				Rectangle rects[4] = {
					new Rectangle(0, 0,
						256, thickness, color),
					new Rectangle(0, 168-thickness,
						256, thickness, color),
					new Rectangle(0, thickness,
						thickness, 168-thickness*2, color),
					new Rectangle(256-thickness, thickness,
						thickness, 168-thickness*2, color)
				};
				loop()
				{
					for(r : rects) // loop through each object
						r->draw(7); // and call its' draw function
					Waitframe();
				}
			}
		}

.. todo::

	- Example of a destructor doing something useful
	- Example of accessing data members of a class externally
	- Example of static functions
	- Details on object arrays being special
