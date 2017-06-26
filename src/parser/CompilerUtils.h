// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H
#define ZSCRIPT_COMPILER_UTILS_H

// Delete all the elements in a vector of pointers.
template<class Element>
void deleteElements(vector<Element*>& container)
{
	for (typename vector<Element*>::iterator it = container.begin();
		 it != container.end(); ++it)
		delete *it;
}

// Delete all the elements in a list of pointers.
template<class Element>
void deleteElements(list<Element*>& container)
{
	for (typename list<Element*>::iterator it = container.begin();
		 it != container.end(); ++it)
		delete *it;
}

#endif
