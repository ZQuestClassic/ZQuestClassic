#include "Backend.h"
#include "GraphicsBackend.h"
#include "MouseBackend.h"
#include "PaletteBackend.h"

bool Backend::initialized_ = false;

GraphicsBackend *Backend::graphics = NULL;
MouseBackend *Backend::mouse = NULL;
PaletteBackend *Backend::palette = NULL;

void Backend::initializeBackend()
{
	if (!initialized_)
	{
		initialized_ = true;
		Backend::graphics = new GraphicsBackend();
		Backend::mouse = new MouseBackend();
		Backend::palette = new PaletteBackend();
	}
}

void Backend::shutdownBackend()
{
	if (initialized_)
	{
		delete Backend::palette;
		delete Backend::mouse;
		delete Backend::graphics;
		initialized_ = false;
	}
}