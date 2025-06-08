#option ON_MISSING_RETURN error
// Is ignored since namespace overrides.
#option LEGACY_ARRAYS error

namespace ns {
	#option LEGACY_ARRAYS warn

	// Should error.
	float fn(){}

	void fn4(int arr) {
		// Should warn.
		arr[0] = 0;
	}
}
