#option ON_MISSING_RETURN warn
#option LEGACY_ARRAYS on

// Note: options defined within the root of imported files
// never impact the importer.
import "misc/namespace_options_import.zs"

// Above option makes this non-returning non-void function warn rather than error.
float fn(){}

// Above option makes this non-returning non-void function warn rather than error.
void fn_legacy_array(int arr) {
	// Default is to error, but above option suppresses that.
	arr[0] = 0;
	// Should warn.
	{
		#option LEGACY_ARRAYS default
		arr[0] = 0;
	}
}

namespace ns {
	// Should warn.
	// Should inherit the root file scope's option, which makes this
	// function declaration warn rather than error.
	// If this error'd instead, that means the default is being used instead of
	// the current file scope; or that the same logical namespace's file scope
	// (from the import) is being used instead.
	float fn2(){}

	void fn3(int arr) {
		// Should not error or warn.
		arr[0] = 0;
		// Should warn.
		{
			#option LEGACY_ARRAYS default
			arr[0] = 0;
		}
	}
}

namespace ns {
	#option ON_MISSING_RETURN error
	#option LEGACY_ARRAYS on

	void fn5(int arr) {
		// Should not error or warn.
		arr[0] = 0;
	}

	// Should error.	
	float fn6(){}
}
