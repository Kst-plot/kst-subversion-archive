add_subdirectory(bin)
add_subdirectory(binnedmap)
add_subdirectory(periodogram)
add_subdirectory(phase)
add_subdirectory(statistics)
add_subdirectory(chop)
add_subdirectory(crossspectrum)
add_subdirectory(syncbin)
add_subdirectory(effective_bandwidth)
add_subdirectory(despike)
add_subdirectory(cumulative_sum)
add_subdirectory(differentiation)
add_subdirectory(shift)
add_subdirectory(discretizing_filters)
add_subdirectory(lfidifference)
add_subdirectory(combine)
add_subdirectory(trim)
add_subdirectory(convert_time)
add_subdirectory(normalization)
add_subdirectory(crosscorrelation_bruteforce)
add_subdirectory(planckUtils)
add_subdirectory(reverse)
add_subdirectory(filters)

IF (KST_HAVE_GSL)
    add_subdirectory(noise_addition)
    add_subdirectory(fits)
    add_subdirectory(fits_nonlinear)
    add_subdirectory(interpolations)
    add_subdirectory(spectral_normalization)
    add_subdirectory(pass_filters)
    add_subdirectory(convolution)
    add_subdirectory(correlation)
ELSE (KST_HAVE_GSL)
    message(STATUS "========== some plugins will not be compiled. Install gsl to add this functionality")
ENDIF (KST_HAVE_GSL)
