Introduction
============

ZScript is the scripting language for extending the capabilities of the ZQuest Classic game engine.

Compiling scripts
-----------------

The first thing to know about ZScript is how to add scripts to your quest. The details of the language
are covered in the following pages, but let's start here.

Let's use this example script:

.. zscript::

	ffc script MyFirstScript
	{
		void run()
		{
			Trace("It's dangerous to go alone! Take this.");
		}
	}

Now, let's add this script to a quest. In the ZQuest Classic editor, open ``ZScript > Compile Script > Edit`` to open
the script buffer. Paste the script, press ``Compile``, then press ``OK``. On the right-hand side, select ``MyFirstScript`` and
press the ``<<`` button to assign it to the first FFC "slot". Finally, press ``OK``.

That's it! This script can now be assigned to any FFC in the editor.

.. tip::

	The ``Quick Compile`` and ``Smart Compile`` options are shortcuts to skip the slot-assignment interface. ``Quick Compile`` will
	update any previously-assigned scripts of the same name for you, and ``Smart Compile`` will do that plus assign new scripts
	to the next available open slot.

Include paths
-------------

To compile additional files, you must use the ``#include`` compiler directive. For example:

.. zscript::

	#include "std.zh"

The compiler needs to know what folders to search in to find the given file. There are a few folders that it looks in by default:
"include", "headers", and "scripts" (all relative to ZC's install folder). Therefore, ZScript libraries that are packaged with the program
can be included without any extra configuration (like for std.zh in the above example).

The list of directories to search is configurable via the ``ZScript > Compile Settings`` dialog (each folder is separated by a semicolon).

.. versionadded:: 2.55.13

	By default, ``<.qst directory>/scripts`` is included for you. So if you place all your scripts inside a folder named "scripts" next to your ``.qst`` file, you won't ever need to modify the include paths.

.. tip::

	While you can directly edit the script buffer to write scripts, it is recommended to instead place just this single line in the
	buffer:

	.. zscript::

		#include "entry.zs"

	where ``entry.zs`` is a text file that includes all the code needed for your quest. For example, your ``entry.zs`` may look like:

	.. zscript::

		#include "std.zh"
		#include "MyCoolScript.zs"
		#include "bosses/AncientMage.zs"

	Place ``entry.zs`` (and any additional scripts you need) in a folder called ``scripts`` next to your ``.qst`` file. If you are using
	2.55.13 or above, this should compile without needing to configure include paths. Otherwise, you'll need to add the path to your
	scripts folder.

	Now you may edit that file in any text editor and use ``ZScript > Compile Script > Compile`` to recompile the scripts in your quest.

Examples
--------

View some curated :ref:`examples<zscript_examples>`.

Visual Studio Code
------------------

.. _vscode:

For an improved programming experience, including error messages, quick access to documentation, and various productivity features,
it is recommended to use Visual Studio Code with the
`ZScript extension <https://marketplace.visualstudio.com/items?itemName=cjamcl.zquest-lsp>`_.

To set this up:

1. Install `VS Code <https://code.visualstudio.com/>`_
2. Open VS Code and find the Extensions tab on the left, search and install the ``zquest-lsp`` extension (or find the install button on the `extension page <https://marketplace.visualstudio.com/items?itemName=cjamcl.zquest-lsp>`_)
3. Open the extension's settings (click the cog > "Settings") and set the "Installation Folder" option to be the same folder
   where ZC is installed

Now ZScript files (``.zh``, ``.zs``, or ``.z``) will have syntax highlighting, error reporting, and various other productivity
features like documentation tooltips and Go-To-Definition lookups (try hovering or right-clicking on variable/function names).

Debugger
--------

.. versionadded:: 3.0

You can use the :ref:`the debugger<debugger>` to debug scripts you write.
