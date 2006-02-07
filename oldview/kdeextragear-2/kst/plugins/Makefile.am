if include_noise_plugins
NOISE_SUBDIR=noise_addition
endif
if include_fits_plugins
FITS_SUBDIR=fits
endif
if include_fitsnonlinear_plugins
FITSNONLINEAR_SUBDIR=fits_nonlinear
endif
if include_interpolations_plugins
INTERPOLATIONS_SUBDIR=interpolations
endif
if include_pass_filters_plugins
PASS_FILTERS_SUBDIR=pass_filters
endif

SUBDIRS=linefit autocorrelation crosscorrelation periodogram phase statistics chop $(NOISE_SUBDIR) $(FITS_SUBDIR) $(FITSNONLINEAR_SUBDIR) $(INTERPOLATIONS_SUBDIR) $(PASS_FILTERS_SUBDIR) editor
