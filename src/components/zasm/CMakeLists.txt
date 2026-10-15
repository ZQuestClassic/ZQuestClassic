add_library(zasm STATIC
	debug_data.cpp
	eval.cpp
	inst_var_remap.cpp
	serialize.cpp
	table.cpp
)
target_link_libraries(zasm PUBLIC base)
# Only because serialize.h includes core/zdefs.h, which a component should not (see the
# layering exceptions in scripts/check_layering.py).
target_link_libraries(zasm PUBLIC allegro_with_legacy)
