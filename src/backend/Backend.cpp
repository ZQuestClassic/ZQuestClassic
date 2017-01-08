#include "Backend.h"
#include "GraphicsBackend.h"

bool Backend::initialized_ = false;

GraphicsBackend *Backend::graphics = NULL;

void Backend::initializeBackend()
{
	if (!initialized_)
	{
		initialized_ = true;
		Backend::graphics = new GraphicsBackend();
	}
}

void Backend::shutdownBackend()
{
	if (initialized_)
	{
		delete Backend::graphics;
		initialized_ = false;
	}
}