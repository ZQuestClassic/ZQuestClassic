Compiler Directives
===================

.. _zslang_directives:

Compiler directives are statements whose purpose is to directly interface
with the compiler in some way. They have a variety of uses.

.. _directive_import:

Importing Files
---------------

Using the `import` or `#include` directive, you can include other ZScript
code files in your project. Normally, only the script buffer of a quest
is compiled- but each of these statements adds a file to be compiled.

Generally, including the same file multiple times has no ill effects-
unless you change the :ref:`HEADER_GUARD option<zslang_options>`.

.. _directive_isincluded:

Checking Imported Files
-----------------------

.. todo::

	`IS_INCLUDED()`

.. _directive_optionval:

Changing Options
----------------

Using the `#option` directive allows you to change compiler options
per-scope. :ref:`More details.<zslang_options>`

.. _directive_repeat:

Repeating Code
--------------

.. todo::

	`repeat()`

.. _directive_assert:

Custom Errors
-------------

.. todo::

	`CONST_ASSERT()`

.. _directive_catch:

Bypassing Warnings and Errors
-----------------------------

.. todo::

	`catch` / `#ignore error` / `#ignore warning`