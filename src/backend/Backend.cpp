#include "Backend.h"
#include "GraphicsBackend.h"
#include "MouseBackend.h"

bool Backend::initialized_ = false;

GraphicsBackend *Backend::graphics = NULL;
MouseBackend *Backend::mouse = NULL;

void Backend::initializeBackend()
{
	if (!initialized_)
	{
		initialized_ = true;
		Backend::graphics = new GraphicsBackend();
		Backend::mouse = new MouseBackend();
	}
}

void Backend::shutdownBackend()
{
	if (initialized_)
	{
		delete Backend::mouse;
		delete Backend::graphics;
		initialized_ = false;
	}
}