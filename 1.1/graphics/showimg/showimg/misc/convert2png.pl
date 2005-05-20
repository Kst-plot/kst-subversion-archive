#!/usr/bin/perl

=NAME

= gimp-ahsv - Apply "Autostetch HSV" effect on a number of JPEG images

= head1 SYNOPSIS

=gimp-ahsv [options] FILE...

=Options:
=  -?, --help
=      --man
=  -v, --verbose

=head1 OPTIONS

=over 4

=item B<-?>

=item B<--help>

=Print a brief help message and exits.

=item B<--man>

=Prints the manual page and exits.

=item B<-v>

=item B<--verbose>

=This option increases the amount of information you given during
=program run. By default, gimp-ahsv works silently.

=head1 DESCRIPTION

=This program applies "Image/Colors/Auto/A­utostetch HSV"
=effect from gimp in batch mode on a number of JPEG images. This effect
=often improves contrast of pictures and improves their overall look.

=head1 BUGS AND LIMITATIONS

=Loading and saving JPEG files decreases quality of the images. For the
=purposes I use this script such quality drop is acceptable. If it is
=not acceptiable for you than use other formats. You should replace
=file_jpeg_load/file_­jpeg_save method calls with other methods from
=file_xxx_load/file_x­xx_save family if you want to use this script with
=other formats.

=Unfortunately even in batch mode gimp requires X server so it is impossible to run this script without it.

=head1 COPYRIGHT

=Copyright (c) 2002 Ilya Martynov. All rights reserved.

=This program is free software.  It may be used, redistributed and/or
=modified under the terms of the Perl Artistic License.

=head1 SEE ALSO

=L<Gimp>

=PDB explorer in gimp
=cut



use strict;
use warnings;

use Pod::Usage;

# configure Getopt::Long to parse argument string in GNU style
use Getopt::Long qw(:config gnu_getopt);

my @files;
my $verbose;

BEGIN {
   # Gimp module insists on parsing @ARGV on its own; for this reason
   # argument string is parsed here

   my %options;

   GetOptions(\%options, 'help|?', 'verbose|v+', 'man');
   pod2usage(-exitval => 0,
             -verbose => 2,
             -output  => \*STDOUT) if $options{man};
   pod2usage(-exitval => 0,
             -output  => \*STDOUT) if $options{help};
   @files = @ARGV;
   pod2usage(2) unless @files;

   $verbose = $options{verbose} || 0;
}




use Gimp qw( :auto );
use Gimp::Fu;

# init Gimp modules
Gimp::set_trace(TRACE_ALL) if $verbose > 1;
Gimp::init;

for my $file (@files) {
   print "Loading $file...\n" if $verbose > 0;
   my $img = eval { Gimp->file_psd_load($file, $file) };

   if($@) {
       # catch file loading errors and skip to the next file
       warn $@;
       next;
   }

   print "Applying effect ...\n" if $verbose > 0;


my $layer = $img->layer_new($img->width, $img->height,
                  RGBA_IMAGE,
                  "first layer",
                  100, NORMAL_MODE);
    $img->add_layer($layer, 0);
    
       my $drawable = $img->merge_visible_layers(0);
  
    
   print "Saving $file ...\n" if $verbose > 0;
   my $quality     = 1.0;
   my $smoothing   = 0;
   my $optimize    = 1;
   my $progressive = 0;
   my $comment     = 'polom';
   my $subsmp      = 0;
   my $baseline    = 1;
   my $restart     = 0;
   my $dst         = 2;
   eval {
#Gimp::set_trace(-1);
       Gimp->file_png_save($img, $drawable, "/tmp/showimgFromPSD.png", "/tmp/showimgFromPSD.png", 0, 0, 0, 0, 0, 0, 0);
#Gimp::set_trace(0);	
   };
   if($@) {
       # catch file saving errors and skip to the next file
       warn $@;
       next;
   }

   # free memory
   $img->delete;
}
