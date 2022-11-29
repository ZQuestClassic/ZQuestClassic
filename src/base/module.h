#ifndef _ZC_MODULE_H_
#define _ZC_MODULE_H_

class ZModule
{
	public:
		bool init(bool d); //bool default
		bool load(bool zquest);
		void debug(); //Print struct data to allegro.log.
	private:
		char n;
};

#endif

