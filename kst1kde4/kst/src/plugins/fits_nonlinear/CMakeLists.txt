add_subdirectory(exponential)
add_subdirectory(exponential_weighted)
add_subdirectory(gaussian)
add_subdirectory(gaussian_weighted)
add_subdirectory(lorentzian)
add_subdirectory(lorentzian_weighted)

if(KST_HAVE_MUPARSER)
    add_subdirectory(general_levenberg_marquardt)
else(KST_HAVE_MUPARSER)
    message(STATUS "========== general Levenberg-Marquardt fit plugin will not be compiled. Install muparser to add this functionality")
endif(KST_HAVE_MUPARSER)

