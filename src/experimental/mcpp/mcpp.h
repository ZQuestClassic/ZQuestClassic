/* mcpp_lib.h: declarations of libmcpp exported (visible) functions */
#ifndef _MCPP_LIB_H
#define _MCPP_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MCPP_OUT_H
	typedef enum {
		MCPP_OUT,                        /* ~= fp_out    */
		MCPP_ERR,                        /* ~= fp_err    */
		MCPP_DBG,                        /* ~= fp_debug  */
		MCPP_NUM_OUTDEST
	} MCPP_OUTDEST;
#endif

#if _WIN32 || _WIN64 || __CYGWIN__ || __CYGWIN64__ || __MINGW32__   \
            || __MINGW64__
#if     DLL_EXPORT || (__CYGWIN__ && PIC)
#define DLL_DECL    __declspec( dllexport)
#elif   DLL_IMPORT
#define DLL_DECL    __declspec( dllimport)
#else
#define DLL_DECL
#endif
#else
#define DLL_DECL
#endif

#define MCPPEXTERN extern

MCPPEXTERN DLL_DECL int     mcpp_lib_main( int argc, char ** argv);
MCPPEXTERN DLL_DECL void    mcpp_reset_def_out_func( void);
MCPPEXTERN DLL_DECL void    mcpp_set_out_func(
                    int (* func_fputc)  ( int c, MCPP_OUTDEST od),
                    int (* func_fputs)  ( const char * s, MCPP_OUTDEST od),
                    int (* func_fprintf)( MCPP_OUTDEST od, const char * format, ...)
                    );
MCPPEXTERN DLL_DECL void    mcpp_use_mem_buffers( int tf);
MCPPEXTERN DLL_DECL char *  mcpp_get_mem_buffer( MCPP_OUTDEST od);


#ifdef __cplusplus
} //extern "C"
#endif

//
//namespace mcpp
//{
//
//	inline int     lib_main( int argc, char ** argv )	{ return mcpp_lib_main( argc, argv ); }
//	inline void    reset_def_out_func( void )			{ mcpp_reset_def_out_func(); }
//	inline void    set_out_func(
//                    int (* func_fputc)  ( int c, OUTDEST od ),
//                    int (* func_fputs)  ( const char * s, OUTDEST od ),
//                    int (* func_fprintf)( OUTDEST od, const char * format, ... )
//					)									{ mcpp_set_out_func( func_fputc, func_fputs, func_fprintf ); }
//	inline void    use_mem_buffers( int tf )			{ mcpp_use_mem_buffers( tf ); }
//	inline char *  get_mem_buffer( OUTDEST od )			{ return mcpp_get_mem_buffer( od ); }
//
//}


#endif  /* _MCPP_LIB_H  */
