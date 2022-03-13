#ifndef ZC_SIGNALS_H
#define ZC_SIGNALS_H

#include <functional>
#include <memory>
#include <utility>

/* A minimal signal class. We could use something like boost::signals2,
 * but that's serious overkill right now.
 *
 * This is EXTREMELY minimal. It assumes it will only be emitter once
 * and that it doesn't need to care about what happens in response.
 */
class Signal
{
public:
	void addListener(std::function<void()> listener)
	{
		listeners.push_back(std::move(listener));
	}

	/* Leaving this in the header on the basis that it'll probably
	 * become a template at some point.
	 */
	void emit()
	{
		for(auto& listener: listeners)
			listener();
	}

private:
	std::vector<std::function<void()>> listeners;
};

#endif
