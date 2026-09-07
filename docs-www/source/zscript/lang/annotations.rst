Annotations
===========

.. _zslang_annotations:

.. _annotations:

Annotations allow setting specific metadata and settings related to single
statements. For example, an annotation might be set on a script to indicate
the author, or set on an `enum` to indicate how much the constants inside
should increment by.

Syntax
------

An annotation is written as an :zs_meta:`@`, the annotation's name, and then its
parameters in parentheses, placed directly before the thing it applies to.
Annotations may take a number of parameters.

Multiple annotations can be applied to the same target, either by listing them
on separate lines, or as a comma-separated list.

.. zscript::

	@Author("EmilyV")
	@InitD0("Speed"), @InitD1("Damage")
	ffc script Example
	{
		void run(int speed, int damage)
		{
			// ...
		}
	}

Available annotations
---------------------

Individual annotations will be explained on the pages they are relevant to.
Below is a list of such pages:

- :ref:`Enums<enum_annotations>`
- :ref:`Smart Loops<loop_annotations>`
- :ref:`Scripts<zslang_scripts>` (including :ref:`exported variables<script_static>`)
