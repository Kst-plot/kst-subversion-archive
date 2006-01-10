if include_piolib
PIOLIB_SUBDIR=planck
endif

if include_fitsio_datasource
FITSIO_SUBDIR=lfiio
FITSIMAGE_SUBDIR=fitsimage
HEALPIX_SUBDIR=healpix
endif

if include_cdf
CDF_SUBDIR=cdf
endif

if include_netcdf
NETCDF_SUBDIR=netcdf
endif

SUBDIRS=ascii qimagesource $(FITSIMAGE_SUBDIR) dirfile frame indirect $(PIOLIB_SUBDIR) $(FITSIO_SUBDIR) $(CDF_SUBDIR) $(NETCDF_SUBDIR) $(HEALPIX_SUBDIR)

