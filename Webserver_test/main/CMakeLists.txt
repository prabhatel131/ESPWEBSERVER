idf_component_register(SRCS "main.c"
		"module_in_main/url_decoding_module.c"
                    INCLUDE_DIRS ".")

idf_build_set_property(COMPILE_OPTIONS "-Wno-format" "-DDEBUG=1" APPEND)                    
