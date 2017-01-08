#ifndef BACKEND_H
#define BACKEND_H

class GraphicsBackend;

class Backend
{
public:
	static GraphicsBackend *graphics;

	static void initializeBackend();
	static void shutdownBackend();

private:
	static bool initialized_;
};

#endif