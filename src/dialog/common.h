#ifndef ZC_DIALOG_COMMON_H
#define ZC_DIALOG_COMMON_H

#ifndef _WIN32
#include <memory>
#endif

#include <gui/list_data.h>
#include "base/zsys.h"

template<typename T>
struct WidgetSetItem
{
	std::shared_ptr<T> widg;
	
	WidgetSetItem<T> *next, *prev;
	
	WidgetSetItem(std::shared_ptr<T>& widg, WidgetSetItem<T> *prev) :
		widg(widg), next(NULL), prev(prev)
	{}
};
template<typename T>
struct WidgetSet
{
	WidgetSetItem<T> *head, *tail;
	std::shared_ptr<T> add(std::shared_ptr<T> widg)
	{
		if(!head)
			head = tail = new WidgetSetItem<T>(widg, NULL);
		else
		{
			tail->next = new WidgetSetItem<T>(widg, tail);
			tail = tail->next;
		}
		++_size;
		return widg;
	}
	void forEach(std::function<void(std::shared_ptr<T>& widg)> func)
	{
		for(WidgetSetItem<T>* node = head; node; node = node->next)
		{
			func(node->widg);
		}
	}
	void forEach(std::function<void(std::shared_ptr<T>& widg,size_t ind)> func)
	{
		size_t ind = 0;
		for(WidgetSetItem<T>* node = head; node; node = node->next)
		{
			func(node->widg, ind++);
		}
	}
	
	std::shared_ptr<T>& operator[](int32_t offset)
	{
		//Assert: No '[]' on an empty WidgetSet
		if(!head) Z_error_fatal("WidgetSet error: [] on empty WidgetSet! Crashing...\n");
		ASSERT(head); //otherwise have no 'std::shared_ptr<T>&' to return...
		if(head==tail) return head->widg;
		//[-1] accesses the tail, further negatives go backwards.
		//If it would over/underflow, it wraps, as a sanity concern.
		bool rev = offset < 0;
		if(offset < 0) offset = -(1+offset);
		WidgetSetItem<T>* node = (rev ? tail : head);
		while(offset--)
		{
			node = (rev ? node->prev : node->next);
			if(!node) node = (rev ? tail : head);
		}
		return node->widg;
	}
	
private:
	size_t _size;
	void deleteAll()
	{
		if(!head) return;
		WidgetSetItem<T>* tmp = head->next;
		while(tmp)
		{
			delete head;
			head = tmp;
			tmp = head->next;
		}
		delete head;
		head = tail = NULL;
	}
public:
	size_t size() const {return _size;}
	WidgetSet() : head(NULL), tail(NULL), _size(0) {}
	~WidgetSet()
	{
		deleteAll();
	}
};


GUI::ListData getItemListData(bool includeNone);
GUI::ListData getStringListData();
GUI::ListData getShopListData();
GUI::ListData getBShopListData();
GUI::ListData getInfoShopListData();

#endif
