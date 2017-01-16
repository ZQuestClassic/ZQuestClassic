#ifndef BACKEND_H
#define BACKEND_H

class GraphicsBackend;
class MouseBackend;
class PaletteBackend;

class Backend
{
public:
	static GraphicsBackend *graphics;
	static MouseBackend *mouse;
	static PaletteBackend *palette;

	static void initializeBackend();
	static void shutdownBackend();

private:
	static bool initialized_;
};

#endif