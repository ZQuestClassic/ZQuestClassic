.. _comments:

Comments
========

Anything inside a comment is ignored by the compiler. There are two styles
of comment in ZScript:

* Line comments - Everything on a line after `//`
* Block comments - Everything between `/*` and :zs_cmnt:`*/`, including across multiple lines

For example:

.. zscript::

	/* This is a long comment that will
	   span across more than one line
	   until it eventually hits the */
	void example(int[] a /* an array */, int b /* not an array */)
	{
		a[0] = b; // an example function
	}

Documentation Comments
----------------------

Comments next to most symbols (anything with an identifier) act as documentation
for that symbol. The text within such comments are displayed to users of the
|VSCode| extension.

These documentation comments are also used to generate web documentation,
such as the documentation available :ref:`on this page<zsdoc_index>`.

Generally, a comment placed above what you want to document is best-
though a comment at the end of the same line as it also works most of the time.

.. zscript::

	/*
		Represents a basic rectangle, and can be
		used to draw it to the screen.
	*/
	class Rectangle
	{
		int x, y;
		int w, h;
		int color; // The color of the rectangle

		/* Draw the rectangle to the screen */
		void draw(int layer)
		{
			// imagine some code here
		}
	}

