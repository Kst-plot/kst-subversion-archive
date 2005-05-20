/***************************************************************************
                          exif.cpp  -  description
                             -------------------
    begin                : Dec 1999
    copyright            : (C) 1999 by Matthias Wandel, 2000 by Richard Groult
    email                : mwandel@sentex.net  rgroult@jalix.org 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include "exif.h"


static unsigned char * LastExifRefd;
static int ExifSettingsLength;
static double FocalplaneXRes;
static double FocalplaneUnits;
static int ExifImageWidth;
static int MotorolaOrder = 0;
//static char * ApplyCommand = NULL;  // Apply this command to all images.
static char * ExifSourceName = NULL;// Extract Exif header from this file, and
static int FilesMatched;
static int TrimExif = FALSE;	    // Cut off exif beyond interesting data.
static int DeleteComments = FALSE;
static int ShowConcise  = FALSE;
static int DoAction	= FALSE;
static Section_t Sections[MAX_SECTIONS];

 //Command line parsing code
static const char * progname;	// program name for error messages
static char * FilterModel = NULL;
static int SectionsRead;
static int HaveAll;
//static int SetFileTime  = FALSE;
//static int RenameToDate = FALSE;
//static char * strftime_args = NULL; // Format for new file name.
static const char * CurrentFile;
static ImageInfo_t ImageInfo;
static int ShowTags;
//static char * ThumbnailName = NULL; // If not NULL, use this string to make up
				  // the filename to store the thumbnail to.

typedef struct {
    unsigned short Tag;
    const char * Desc;
}TagTable_t;
//--------------------------------------------------------------------------
// Table of Jpeg encoding process names

#define M_SOF0  0xC0            // Start Of Frame N
#define M_SOF1  0xC1            // N indicates which compression process
#define M_SOF2  0xC2            // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5            // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8            // Start Of Image (beginning of datastream)
#define M_EOI   0xD9            // End Of Image (end of datastream)
#define M_SOS   0xDA            // Start Of Scan (begins compressed data)
#define M_JFIF  0xE0            // Jfif marker
#define M_EXIF  0xE1            // Exif marker
#define M_COM   0xFE            // COMment 
static TagTable_t ProcessTable[] = {
    { M_SOF0,   "Baseline"},
    { M_SOF1,   "Extended sequential"},
    { M_SOF2,   "Progressive"},
    { M_SOF3,   "Lossless"},
    { M_SOF5,   "Differential sequential"},
    { M_SOF6,   "Differential progressive"},
    { M_SOF7,   "Differential lossless"},
    { M_SOF9,   "Extended sequential, arithmetic coding"},
    { M_SOF10,  "Progressive, arithmetic coding"},
    { M_SOF11,  "Lossless, arithmetic coding"},
    { M_SOF13,  "Differential sequential, arithmetic coding"},
    { M_SOF14,  "Differential progressive, arithmetic coding"},
    { M_SOF15,  "Differential lossless, arithmetic coding"},
    { 0,        "Unknown"}
};


//--------------------------------------------------------------------------
// Describes format descriptor
static int BytesPerFormat[] = {0,1,1,2,4,8,1,1,2,4,8,4,8};
#define NUM_FORMATS 12

#define FMT_BYTE       1 
#define FMT_STRING     2
#define FMT_USHORT     3
#define FMT_ULONG      4
#define FMT_URATIONAL  5
#define FMT_SBYTE      6
#define FMT_UNDEFINED  7
#define FMT_SSHORT     8
#define FMT_SLONG      9
#define FMT_SRATIONAL 10
#define FMT_SINGLE    11
#define FMT_DOUBLE    12

//--------------------------------------------------------------------------
// Describes tag values

#define TAG_EXIF_OFFSET       0x8769
#define TAG_INTEROP_OFFSET    0xa005

#define TAG_MAKE              0x010F
#define TAG_MODEL             0x0110

#define TAG_EXPOSURETIME      0x829A
#define TAG_FNUMBER           0x829D

#define TAG_SHUTTERSPEED      0x9201
#define TAG_APERTURE          0x9202
#define TAG_MAXAPERTURE       0x9205
#define TAG_FOCALLENGTH       0x920A

#define TAG_DATETIME_ORIGINAL 0x9003
#define TAG_USERCOMMENT       0x9286

#define TAG_SUBJECT_DISTANCE  0x9206
#define TAG_FLASH             0x9209

#define TAG_FOCALPLANEXRES    0xa20E
#define TAG_FOCALPLANEUNITS   0xa210
#define TAG_EXIF_IMAGEWIDTH   0xA002
#define TAG_EXIF_IMAGELENGTH  0xA003

// the following is added 05-jan-2001 vcs
#define TAG_EXPOSURE_BIAS     0x9204
#define TAG_WHITEBALANCE      0x9208
#define TAG_METERING_MODE     0x9207
#define TAG_EXPOSURE_PROGRAM  0x8822
#define TAG_ISO_EQUIVALENT    0x8827
#define TAG_COMPRESSION_LEVEL 0x9102

#define TAG_THUMBNAIL_OFFSET  0x0201
#define TAG_THUMBNAIL_LENGTH  0x0202

static TagTable_t TagTable[] = {
  {   0x100,   "ImageWidth"},
  {   0x101,   "ImageLength"},
  {   0x102,   "BitsPerSample"},
  {   0x103,   "Compression"},
  {   0x106,   "PhotometricInterpretation"},
  {   0x10A,   "FillOrder"},
  {   0x10D,   "DocumentName"},
  {   0x10E,   "ImageDescription"},
  {   0x10F,   "Make"},
  {   0x110,   "Model"},
  {   0x111,   "StripOffsets"},
  {   0x112,   "Orientation"},
  {   0x115,   "SamplesPerPixel"},
  {   0x116,   "RowsPerStrip"},
  {   0x117,   "StripByteCounts"},
  {   0x11A,   "XResolution"},
  {   0x11B,   "YResolution"},
  {   0x11C,   "PlanarConfiguration"},
  {   0x128,   "ResolutionUnit"},
  {   0x12D,   "TransferFunction"},
  {   0x131,   "Software"},
  {   0x132,   "DateTime"},
  {   0x13B,   "Artist"},
  {   0x13E,   "WhitePoint"},
  {   0x13F,   "PrimaryChromaticities"},
  {   0x156,   "TransferRange"},
  {   0x200,   "JPEGProc"},
  {   0x201,   "ThumbnailOffset"},
  {   0x202,   "ThumbnailLength"},
  {   0x211,   "YCbCrCoefficients"},
  {   0x212,   "YCbCrSubSampling"},
  {   0x213,   "YCbCrPositioning"},
  {   0x214,   "ReferenceBlackWhite"},
  {   0x828D,  "CFARepeatPatternDim"},
  {   0x828E,  "CFAPattern"},
  {   0x828F,  "BatteryLevel"},
  {   0x8298,  "Copyright"},
  {   0x829A,  "ExposureTime"},
  {   0x829D,  "FNumber"},
  {   0x83BB,  "IPTC/NAA"},
  {   0x8769,  "ExifOffset"},
  {   0x8773,  "InterColorProfile"},
  {   0x8822,  "ExposureProgram"},
  {   0x8824,  "SpectralSensitivity"},
  {   0x8825,  "GPSInfo"},
  {   0x8827,  "ISOSpeedRatings"},
  {   0x8828,  "OECF"},
  {   0x9000,  "ExifVersion"},
  {   0x9003,  "DateTimeOriginal"},
  {   0x9004,  "DateTimeDigitized"},
  {   0x9101,  "ComponentsConfiguration"},
  {   0x9102,  "CompressedBitsPerPixel"},
  {   0x9201,  "ShutterSpeedValue"},
  {   0x9202,  "ApertureValue"},
  {   0x9203,  "BrightnessValue"},
  {   0x9204,  "ExposureBiasValue"},
  {   0x9205,  "MaxApertureValue"},
  {   0x9206,  "SubjectDistance"},
  {   0x9207,  "MeteringMode"},
  {   0x9208,  "LightSource"},
  {   0x9209,  "Flash"},
  {   0x920A,  "FocalLength"},
  {   0x927C,  "MakerNote"},
  {   0x9286,  "UserComment"},
  {   0x9290,  "SubSecTime"},
  {   0x9291,  "SubSecTimeOriginal"},
  {   0x9292,  "SubSecTimeDigitized"},
  {   0xA000,  "FlashPixVersion"},
  {   0xA001,  "ColorSpace"},
  {   0xA002,  "ExifImageWidth"},
  {   0xA003,  "ExifImageLength"},
  {   0xA005,  "InteroperabilityOffset"},
  {   0xA20B,  "FlashEnergy"},                 // 0x920B in TIFF/EP
  {   0xA20C,  "SpatialFrequencyResponse"},  // 0x920C    -  -
  {   0xA20E,  "FocalPlaneXResolution"},     // 0x920E    -  -
  {   0xA20F,  "FocalPlaneYResolution"},      // 0x920F    -  -
  {   0xA210,  "FocalPlaneResolutionUnit"},  // 0x9210    -  -
  {   0xA214,  "SubjectLocation"},             // 0x9214    -  -
  {   0xA215,  "ExposureIndex"},            // 0x9215    -  -
  {   0xA217,  "SensingMethod"},            // 0x9217    -  -
  {   0xA300,  "FileSource"},
  {   0xA301,  "SceneType"},
  {      0, NULL}
} ;



//--------------------------------------------------------------------------
// Error exit handler
//--------------------------------------------------------------------------
void ErrExit(const char * msg)
{
    fprintf(stderr,"Error : %s\n", msg);
    
}
//--------------------------------------------------------------------------
// Read image data.
//--------------------------------------------------------------------------
static int ReadJpegFile(const char * FileName, ReadMode_t ReadMode)
{
    FILE * infile;
    int ret;

    infile = fopen(FileName, "rb"); // Unix ignores 'b', windows needs it.

    if (infile == NULL) {
        fprintf(stderr, "%s: can't open '%s'\n", progname, FileName);
        return FALSE;
    }

    // Scan the JPEG headers.
    ret = ReadJpegSections(infile, ReadMode);
    if (!ret){
        printf("Not JPEG: %s\n",FileName);
    }

    fclose(infile);

    if (ret == FALSE){
        DiscardData();
    }
    return ret;
}
//--------------------------------------------------------------------------
// check if this file should be skipped based on contents.
//--------------------------------------------------------------------------
static int CheckFileSkip(void)
{
    // I sometimes add code here to only process images based on certain
    // criteria - for example, only to convert non progressive jpegs to progressives, etc..

    if (FilterModel){
        // Filtering processing by camera model.
        if (strstr(ImageInfo.CameraModel, FilterModel) == NULL){
            return TRUE;

        }
    }
    return FALSE;
}
//--------------------------------------------------------------------------
// Discard everything but the exif and comment sections.
//--------------------------------------------------------------------------
static void DiscardAllButExif(void)
{
    Section_t ExifKeeper;
    Section_t CommentKeeper;
    int a;

    memset(&ExifKeeper, 0, sizeof(ExifKeeper));
    memset(&CommentKeeper, 0, sizeof(ExifKeeper));

    for (a=0;a<SectionsRead;a++){
        if (Sections[a].Type == M_EXIF && ExifKeeper.Type == 0){
            ExifKeeper = Sections[a];
        }else if (Sections[a].Type == M_COM && CommentKeeper.Type == 0){
            CommentKeeper = Sections[a];
        }else{
            free(Sections[a].Data);
        }
    }
    SectionsRead = 0;
    if (ExifKeeper.Type){
        Sections[SectionsRead++] = ExifKeeper;
    }
    if (CommentKeeper.Type){
        Sections[SectionsRead++] = CommentKeeper;
    }
}    
//--------------------------------------------------------------------------
// Apply the specified command to the jpeg file.
//--------------------------------------------------------------------------
/*
static void DoCommand(const char * FileName)
{
    int a,e;
    char ExecString[400];
    char TempName[200];
    int TempUsed = FALSE;

    e = 0;

    // Make a temporary file in the destination directory by changing last char.
    strcpy(TempName, FileName);
    a = strlen(TempName)-1;
    TempName[a] = TempName[a] == 't' ? 'z' : 't';

    // Build the exec string.  %i and %o in the exec string get replaced by input and output files.
    for (a=0;;a++){
        if (ApplyCommand[a] == '&'){
            if (ApplyCommand[a+1] == 'i'){
                // Input file.
                if (strstr(FileName, " ")){
                    e += sprintf(ExecString+e, "\"%s\"",FileName);
                }else{
                    // No need for quoting (that way I can put a relative path in front)
                    e += sprintf(ExecString+e, "%s",FileName);
                }
                a += 1;
                continue;
            }
            if (ApplyCommand[a+1] == 'o'){
                // Needs an output file distinct from the input file.
                e += sprintf(ExecString+e, "\"%s\"",TempName);
                a += 1;
                TempUsed = TRUE;
                unlink(TempName);// Remove any pre-existing temp file
                continue;
            }
        }
        ExecString[e++] = ApplyCommand[a];
        if (ApplyCommand[a] == 0) break;
    }

    printf("Cmd:%s\n",ExecString);

    errno = 0;
    a = system(ExecString);

    if (a || errno){
        // A command can however fail without errno getting set or system returning an error.
        if (errno) perror("system");
        ErrExit("Problem executing specified command");
    }

    if (TempUsed){
        // Don't delete original file until we know a new one was created by the command.
        struct stat dummy;
        if (stat(TempName, &dummy) == 0){
            unlink(FileName);
            rename(TempName, FileName);
        }else{
            ErrExit("specified command did not produce expected output file");
        }
    }
}
*/
//--------------------------------------------------------------------------
// Subsititute original name for '&i' if present in specified name.
// This to allow specifying relative names when manipulating multiple files.
//--------------------------------------------------------------------------
static void RelativeName(char * OutFileName, const char * NamePattern, const char * OrigName)
{
    // If the filename contains substring "&i", then substitute the 
    // filename for that.  This gives flexibility in terms of processing
    // multiple files at a time.
    char * Subst;
    Subst = strstr(NamePattern, "&i");
    if (Subst){
        strncpy(OutFileName, NamePattern, Subst-NamePattern);
        OutFileName[Subst-NamePattern] = 0;
        strncat(OutFileName, OrigName, _MAX_PATH);
        strncat(OutFileName, Subst+2, _MAX_PATH);
    }else{
        strcpy(OutFileName, NamePattern); 
    }
}
//--------------------------------------------------------------------------
// Parse the marker stream until SOS or EOI is seen;
//--------------------------------------------------------------------------
int ReadJpegSections (FILE * infile, ReadMode_t ReadMode)
{
    int a;
    int HaveCom = FALSE;

    a = fgetc(infile);


    if (a != 0xff || fgetc(infile) != M_SOI){
        return FALSE;
    }
    for(;SectionsRead < MAX_SECTIONS-1;){
        int itemlen;
        int marker = 0;
        int ll,lh, got;
        uchar * Data;

        for (a=0;a<7;a++){
            marker = fgetc(infile);
            if (marker != 0xff) break;

            if (a >= 6){

                printf("too many padding bytes\n");

                return FALSE;

            }
        }

        if (marker == 0xff){
            // 0xff is legal padding, but if we get that many, something's wrong.
            ErrExit("too many padding bytes!");
	    return 0;
        }

        Sections[SectionsRead].Type = marker;
  
        // Read the length of the section.
        lh = fgetc(infile);
        ll = fgetc(infile);

        itemlen = (lh << 8) | ll;

        if (itemlen < 2){
            ErrExit("invalid marker");
	    return 0;
        }

        Sections[SectionsRead].Size = itemlen;

        Data = (uchar *)malloc(itemlen+1); // Add 1 to allow sticking a 0 at the end.
        if (Data == NULL){
            ErrExit("Could not allocate memory");
	    return 0;
        }
        Sections[SectionsRead].Data = Data;

        // Store first two pre-read bytes.
        Data[0] = (uchar)lh;
        Data[1] = (uchar)ll;

        got = fread(Data+2, 1, itemlen-2, infile); // Read the whole section.
        if (got != itemlen-2){
            ErrExit("reading from file");
	    return 0;
        }
        SectionsRead += 1;

        switch(marker){

            case M_SOS:   // stop before hitting compressed data 
                // If reading entire image is requested, read the rest of the data.
                if (ReadMode & READ_IMAGE){
                    int cp, ep, size;
                    // Determine how much file is left.
                    cp = ftell(infile);
                    fseek(infile, 0, SEEK_END);
                    ep = ftell(infile);
                    fseek(infile, cp, SEEK_SET);

                    size = ep-cp;
                    Data = (uchar *)malloc(size);
                    if (Data == NULL){
                        ErrExit("could not allocate data for entire image");
			return 0;
                    }

                    got = fread(Data, 1, size, infile);
                    if (got != size){
                        ErrExit("could not read the rest of the image");
			return 0;
                    }

                    Sections[SectionsRead].Data = Data;
                    Sections[SectionsRead].Size = size;
                    Sections[SectionsRead].Type = PSEUDO_IMAGE_MARKER;
                    SectionsRead ++;
                    HaveAll = 1;
                }
                return TRUE;

            case M_EOI:   // in case it's a tables-only JPEG stream
                printf("No image in jpeg!\n");
                return FALSE;

            case M_COM: // Comment section
                if (HaveCom || ((ReadMode & READ_EXIF) == 0)){
                    // Discard this section.
                    free(Sections[--SectionsRead].Data);
                }else{
                    process_COM(Data, itemlen);
                    HaveCom = TRUE;
                }
                break;

            case M_JFIF:
                // Regular jpegs always have this tag, exif images have the exif
                // marker instead, althogh ACDsee will write images with both markers.
                // this program will re-create this marker on absence of exif marker.
                // hence no need to keep the copy from the file.
                free(Sections[--SectionsRead].Data);
                break;

            case M_EXIF:
                // Seen files from some 'U-lead' software with Vivitar scanner
                // that uses marker 31 for non exif stuff.  Thus make sure 
                // it says 'Exif' in the section before treating it as exif.
                if ((ReadMode & READ_EXIF) && memcmp(Data+2, "Exif", 4) == 0){
                    process_EXIF((uchar *)Data, itemlen);
                }else{
                    // Discard this section.
                    free(Sections[--SectionsRead].Data);
                }
                break;

            case M_SOF0: 
            case M_SOF1: 
            case M_SOF2: 
            case M_SOF3: 
            case M_SOF5: 
            case M_SOF6: 
            case M_SOF7: 
            case M_SOF9: 
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
                process_SOFn(Data, marker);
                break;
            default:
                // Skip any other sections.
                if (ShowTags){
                    printf("Jpeg section marker 0x%02x size %d\n",marker, itemlen);
                }
                break;
        }
    }
    return TRUE;
}
//--------------------------------------------------------------------------
// Discard read data.
//--------------------------------------------------------------------------
void DiscardData(void)
{
    int a;
    for (a=0;a<SectionsRead;a++){
        free(Sections[a].Data);
    }
    memset(&ImageInfo, 0, sizeof(ImageInfo));
    SectionsRead = 0;
    HaveAll = 0;
}
//--------------------------------------------------------------------------
// Remove exif thumbnail
//--------------------------------------------------------------------------
static int TrimExifFunc(void)
{
    int a;
    for (a=0;a<SectionsRead-1;a++){
        if (Sections[a].Type == M_EXIF){
            // Truncate the thumbnail section of the exif.
            unsigned int Newsize = GetExifNonThumbnailSize();
            if (Sections[a].Size == Newsize) return FALSE; // Thumbnail already gonne.
            Sections[a].Size = Newsize;
            Sections[a].Data[0] = (uchar)(Newsize >> 8);
            Sections[a].Data[1] = (uchar)Newsize;
            return TRUE;
        }
    }
    // Not an exif image.  Can't remove exif thumbnail.
    return FALSE;
}

//--------------------------------------------------------------------------
// Remove comments
//--------------------------------------------------------------------------
static int RemoveComments(void)
{
    int a;
    for (a=0;a<SectionsRead-1;a++){
        if (Sections[a].Type == M_COM){
            // Free up this section
            free (Sections[a].Data);
            // Move succeding sections back by one to close space in array.
            memmove(Sections+a, Sections+a+1, sizeof(Section_t) * (SectionsRead-a));
            SectionsRead -= 1;
            return TRUE;
        }
    }
    return FALSE;
}


//--------------------------------------------------------------------------
// Write image data back to disk.
//--------------------------------------------------------------------------
static void WriteJpegFile(const char * FileName)
{
    FILE * outfile;
    int a;
    fprintf(stderr,"writing %s\n", FileName);
    if (!HaveAll){
        ErrExit("Can't write back - didn't read all");
	return;
    }

    outfile = fopen(FileName,"wb");
    if (outfile == NULL){
        ErrExit("Could not open file for write");
	return;
    }

    // Initial static jpeg marker.
    fputc(0xff,outfile);
    fputc(0xd8,outfile);
    
    if (Sections[0].Type != M_EXIF && Sections[0].Type != M_JFIF){
        // The image must start with an exif or jfif marker.  If we threw those away, create one.
        static uchar JfifHead[18] = {
            0xff, M_JFIF,
            0x00, 0x10, 'J' , 'F' , 'I' , 'F' , 0x00, 0x01, 
            0x01, 0x01, 0x01, 0x2C, 0x01, 0x2C, 0x00, 0x00 
        };
        fwrite(JfifHead, 18, 1, outfile);
    }

    // Write all the misc sections
    for (a=0;a<SectionsRead-1;a++){
        fputc(0xff,outfile);
        fputc(Sections[a].Type, outfile);
        fwrite(Sections[a].Data, Sections[a].Size, 1, outfile);
    }

    // Write the remaining image data.
    fwrite(Sections[a].Data, Sections[a].Size, 1, outfile);
       
    fclose(outfile);
}



//--------------------------------------------------------------------------
// Convert a 16 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
static int Get16u(void * Short)
{
    if (MotorolaOrder){
        return (((uchar *)Short)[0] << 8) | ((uchar *)Short)[1];
    }else{
        return (((uchar *)Short)[1] << 8) | ((uchar *)Short)[0];
    }
}

//--------------------------------------------------------------------------
// Convert a 32 bit signed value from file's native byte order
//--------------------------------------------------------------------------
static int Get32s(void * Long)
{
    if (MotorolaOrder){
        return  ((( char *)Long)[0] << 24) | (((uchar *)Long)[1] << 16)
              | (((uchar *)Long)[2] << 8 ) | (((uchar *)Long)[3] << 0 );
    }else{
        return  ((( char *)Long)[3] << 24) | (((uchar *)Long)[2] << 16)
              | (((uchar *)Long)[1] << 8 ) | (((uchar *)Long)[0] << 0 );
    }
}

//--------------------------------------------------------------------------
// Convert a 32 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
static unsigned Get32u(void * Long)
{
    return (unsigned)Get32s(Long) & 0xffffffff;
}

//--------------------------------------------------------------------------
// Display a number as one of its many formats
//--------------------------------------------------------------------------
static void PrintFormatNumber(void * ValuePtr, int Format, int ByteCount)
{
    int a;
    switch(Format){
        case FMT_SBYTE:
        case FMT_BYTE:      printf("%02x ",*(uchar *)ValuePtr);             break;
        case FMT_USHORT:    printf("%d\n",Get16u(ValuePtr));                break;
        case FMT_ULONG:     
        case FMT_SLONG:     printf("%d\n",Get32s(ValuePtr));                break;
        case FMT_SSHORT:    printf("%hd\n",(signed short)Get16u(ValuePtr)); break;
        case FMT_URATIONAL:
        case FMT_SRATIONAL: 
           printf("%d/%d\n",Get32s(ValuePtr), Get32s(4+(char *)ValuePtr)); break;

        case FMT_SINGLE:    printf("%f\n",(double)*(float *)ValuePtr);   break;
        case FMT_DOUBLE:    printf("%f\n",*(double *)ValuePtr);          break;
        default: 
            printf("Unknown format %d:", Format);
            for (a=0;a<ByteCount && a<16;a++){
                printf("%02x", ((uchar *)ValuePtr)[a]);
            }
            printf("\n");
                    
            
    }
}


//--------------------------------------------------------------------------
// Evaluate number, be it int, rational, or float from directory.
//--------------------------------------------------------------------------
static double ConvertAnyFormat(void * ValuePtr, int Format)
{
    double Value;
    Value = 0;

    switch(Format){
        case FMT_SBYTE:     Value = *(signed char *)ValuePtr;  break;
        case FMT_BYTE:      Value = *(uchar *)ValuePtr;        break;

        case FMT_USHORT:    Value = Get16u(ValuePtr);          break;

        case FMT_ULONG:     Value = Get32u(ValuePtr);          break;

        case FMT_URATIONAL:
        case FMT_SRATIONAL: 
            {
                int Num,Den;
                Num = Get32s(ValuePtr);
                Den = Get32s(4+(char *)ValuePtr);
                if (Den == 0){
                    Value = 0;
                }else{
                    Value = (double)Num/Den;
                }
                break;
            }

        case FMT_SSHORT:    Value = (signed short)Get16u(ValuePtr);  break;
        case FMT_SLONG:     Value = Get32s(ValuePtr);                break;

        // Not sure if this is correct (never seen float used in Exif format)
        case FMT_SINGLE:    Value = (double)*(float *)ValuePtr;      break;
        case FMT_DOUBLE:    Value = *(double *)ValuePtr;             break;
    }
    return Value;
}

//--------------------------------------------------------------------------
// Process one of the nested EXIF directories.
//--------------------------------------------------------------------------
static void ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, unsigned ExifLength)
{
    int de;
    int a;
    int NumDirEntries;
    unsigned ThumbnailOffset = 0;
    unsigned ThumbnailSize = 0;

    NumDirEntries = Get16u(DirStart);

    if ((DirStart+2+NumDirEntries*12) > (OffsetBase+ExifLength)){
        ErrExit("Illegally sized directory");
	return;
    }

    if (ShowTags){
        printf("Directory with %d entries\n",NumDirEntries);
    }

    for (de=0;de<NumDirEntries;de++){
        int Tag, Format, Components;
        unsigned char * ValuePtr;
        int ByteCount;
        char * DirEntry;
        DirEntry = (char *)DirStart+2+12*de;

        Tag = Get16u(DirEntry);
        Format = Get16u(DirEntry+2);
        Components = Get32u(DirEntry+4);

        if ((Format-1) >= NUM_FORMATS) {
            // (-1) catches illegal zero case as unsigned underflows to positive large.
            ErrExit("Illegal format code in EXIF dir");
	    return;
        }

        ByteCount = Components * BytesPerFormat[Format];

        if (ByteCount > 4){
            unsigned OffsetVal;
            OffsetVal = Get32u(DirEntry+8);
            // If its bigger than 4 bytes, the dir entry contains an offset.
            if (OffsetVal+ByteCount > ExifLength){
                // Bogus pointer offset and / or bytecount value
                printf("Offset %d bytes %d ExifLen %d\n",OffsetVal, ByteCount, ExifLength);

                ErrExit("Illegal pointer offset value in EXIF");
		return;
            }
            ValuePtr = OffsetBase+OffsetVal;
        }else{
            // 4 bytes or less and value is in the dir entry itself
            ValuePtr = (unsigned char *)DirEntry+8;
        }

        if (LastExifRefd < ValuePtr+ByteCount){
            // Keep track of last byte in the exif header that was actually referenced.
            // That way, we know where the discardable thumbnail data begins.
            LastExifRefd = ValuePtr+ByteCount;
        }

        if (ShowTags){
            // Show tag name
            for (a=0;;a++){
                if (TagTable[a].Tag == 0){
                    printf("  Unknown Tag %04x Value = ", Tag);
                    break;
                }
                if (TagTable[a].Tag == Tag){
                    printf("    %s = ",TagTable[a].Desc);
                    break;
                }
            }

            // Show tag value.
            switch(Format){

                case FMT_UNDEFINED:
                    // Undefined is typically an ascii string.

                case FMT_STRING:
                    // String arrays printed without function call (different from int arrays)
                    printf("\"");
                    for (a=0;a<ByteCount;a++){
                        if (isprint((ValuePtr)[a])){
                            putchar((ValuePtr)[a]);
                        }
                    }
                    printf("\"\n");
                    break;

                default:
                    // Handle arrays of numbers later (will there ever be?)
                    PrintFormatNumber(ValuePtr, Format, ByteCount);
            }
        }

        // Extract useful components of tag
        switch(Tag){

            case TAG_MAKE:
                strncpy(ImageInfo.CameraMake, (const char*)ValuePtr, 31);
                break;

            case TAG_MODEL:
                strncpy(ImageInfo.CameraModel, (const char*)ValuePtr, 39);
                break;

            case TAG_DATETIME_ORIGINAL:
                strncpy(ImageInfo.DateTime, (const char*)ValuePtr, 19);
                break;

            case TAG_USERCOMMENT:
                // Olympus has this padded with trailing spaces.  Remove these first.
                for (a=ByteCount;;){
                    a--;
                    if ((ValuePtr)[a] == ' '){
                        (ValuePtr)[a] = '\0';
                    }else{
                        break;
                    }
                    if (a == 0) break;
                }

                // Copy the comment
                if (memcmp(ValuePtr, "ASCII",5) == 0){
                    for (a=5;a<10;a++){
                        int c;
                        c = (ValuePtr)[a];
                        if (c != '\0' && c != ' '){
                            strncpy(ImageInfo.Comments, (const char*)(a+ValuePtr), 199);
                            break;
                        }
                    }
                    
                }else{
                    strncpy(ImageInfo.Comments, (const char*)ValuePtr, 199);
                }
                break;

            case TAG_FNUMBER:
                // Simplest way of expressing aperture, so I trust it the most.
                // (overwrite previously computd value if there is one)
                ImageInfo.ApertureFNumber = (float)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_APERTURE:
            case TAG_MAXAPERTURE:
                // More relevant info always comes earlier, so only use this field if we don't 
                // have appropriate aperture information yet.
                if (ImageInfo.ApertureFNumber == 0){
                    ImageInfo.ApertureFNumber 
                        = (float)exp(ConvertAnyFormat(ValuePtr, Format)*log(2.0)*0.5);
                }
                break;

            case TAG_FOCALLENGTH:
                // Nice digital cameras actually save the focal length as a function
                // of how farthey are zoomed in.
                ImageInfo.FocalLength = (float)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_SUBJECT_DISTANCE:
                // Inidcates the distacne the autofocus camera is focused to.
                // Tends to be less accurate as distance increases.
                ImageInfo.Distance = (float)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_EXPOSURETIME:
                // Simplest way of expressing exposure time, so I trust it most.
                // (overwrite previously computd value if there is one)
                ImageInfo.ExposureTime = (float)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_SHUTTERSPEED:
                // More complicated way of expressing exposure time, so only use
                // this value if we don't already have it from somewhere else.
                if (ImageInfo.ExposureTime == 0){
                    ImageInfo.ExposureTime 
                        = (float)(1/exp(ConvertAnyFormat(ValuePtr, Format)*log(2.0)));
                }
                break;

            case TAG_FLASH:
                if (ConvertAnyFormat(ValuePtr, Format)){
                    ImageInfo.FlashUsed = 1;
                }
                break;

            case TAG_EXIF_IMAGELENGTH:
            case TAG_EXIF_IMAGEWIDTH:
                // Use largest of height and width to deal with images that have been
                // rotated to portrait format.
                a = (int)ConvertAnyFormat(ValuePtr, Format);
                if (ExifImageWidth < a) ExifImageWidth = a;
                break;

            case TAG_FOCALPLANEXRES:
                FocalplaneXRes = ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_FOCALPLANEUNITS:
                switch((int)ConvertAnyFormat(ValuePtr, Format)){
                    case 1: FocalplaneUnits = 25.4; break; // inch
                    case 2: 
                        // According to the information I was using, 2 means meters.
                        // But looking at the Cannon powershot's files, inches is the only
                        // sensible value.
                        FocalplaneUnits = 25.4;
                        break;

                    case 3: FocalplaneUnits = 10;   break;  // centimeter
                    case 4: FocalplaneUnits = 1;    break;  // milimeter
                    case 5: FocalplaneUnits = .001; break;  // micrometer
                }
                break;

                // Remaining cases contributed by: Volker C. Schoech (schoech@gmx.de)

            case TAG_EXPOSURE_BIAS:
                ImageInfo.ExposureBias = (float)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_WHITEBALANCE:
                ImageInfo.Whitebalance = (int)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_METERING_MODE:
                ImageInfo.MeteringMode = (int)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_EXPOSURE_PROGRAM:
                ImageInfo.ExposureProgram = (int)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_ISO_EQUIVALENT:
                ImageInfo.ISOequivalent = (int)ConvertAnyFormat(ValuePtr, Format);
                if ( ImageInfo.ISOequivalent < 50 ) ImageInfo.ISOequivalent *= 200;
                break;

            case TAG_COMPRESSION_LEVEL:
                ImageInfo.CompressionLevel = (int)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_THUMBNAIL_OFFSET:
                ThumbnailOffset = (unsigned)ConvertAnyFormat(ValuePtr, Format);
                break;

            case TAG_THUMBNAIL_LENGTH:
                ThumbnailSize = (unsigned)ConvertAnyFormat(ValuePtr, Format);
                break;

        }

        if (Tag == TAG_EXIF_OFFSET || Tag == TAG_INTEROP_OFFSET){
            unsigned char * SubdirStart;
            SubdirStart = OffsetBase + Get32u(ValuePtr);
            if (SubdirStart < OffsetBase || SubdirStart > OffsetBase+ExifLength){
                ErrExit("Illegal subdirectory link");
		return;
            }
            ProcessExifDir(SubdirStart, OffsetBase, ExifLength);
            continue;
        }
    }


    {
        // In addition to linking to subdirectories via exif tags, 
        // ther's also a potential link to another directory at the end of each
        // directory.  this has got to be the result of a comitee!
        unsigned char * SubdirStart;
        unsigned Offset;
        Offset = Get16u(DirStart+2+12*NumDirEntries);
        if (Offset){
            SubdirStart = OffsetBase + Offset;
            if (SubdirStart < OffsetBase || SubdirStart > OffsetBase+ExifLength){
                ErrExit("Illegal subdirectory link");
		return;
            }
            ProcessExifDir(SubdirStart, OffsetBase, ExifLength);
        }
    }


    if (ThumbnailSize && ThumbnailOffset){
        if (ThumbnailSize + ThumbnailOffset <= ExifLength){
            // The thumbnail pointer appears to be valid.  Store it.
            ImageInfo.ThumbnailPointer = OffsetBase + ThumbnailOffset;
            ImageInfo.ThumbnailSize = ThumbnailSize;

            if (ShowTags){
                printf("Thumbnail size: %d bytes\n",ThumbnailSize);
            }
        }
    }
}
//--------------------------------------------------------------------------
// Process a COM marker.
// We want to print out the marker contents as legible text;
// we must guard against random junk and varying newline representations.
//--------------------------------------------------------------------------
void process_COM (const uchar * Data, int length)
{
    int ch;
    char Comment[MAX_COMMENT+1];
    int nch;
    int a;

    nch = 0;

    if (length > MAX_COMMENT) length = MAX_COMMENT; // Truncate if it won't fit in our structure.

    for (a=2;a<length;a++){
        ch = Data[a];

        if (ch == '\r' && Data[a+1] == '\n') continue; // Remove cr followed by lf.

        if (isprint(ch) || ch == '\n' || ch == '\t'){
            Comment[nch++] = (char)ch;
        }else{
            Comment[nch++] = '?';
        }
    }

    Comment[nch] = '\0'; // Null terminate

    if (ShowTags){
        printf("COM marker comment: %s\n",Comment);
    }

    strcpy(ImageInfo.Comments,Comment);
}

 
//--------------------------------------------------------------------------
// Process a SOFn marker.  This is useful for the image dimensions
//--------------------------------------------------------------------------
void process_SOFn (const uchar * Data, int marker)
{
    int data_precision, num_components;

    data_precision = Data[2];
    ImageInfo.Height = Get16m(Data+3);
    ImageInfo.Width = Get16m(Data+5);
    num_components = Data[7];

    if (num_components == 3){
        ImageInfo.IsColor = 1;
    }else{
        ImageInfo.IsColor = 0;
    }

    ImageInfo.Process = marker;

    if (ShowTags){
        printf("JPEG image is %uw * %uh, %d color components, %d bits per sample\n",
                   ImageInfo.Width, ImageInfo.Height, num_components, data_precision);
    }
}

//--------------------------------------------------------------------------
// Get 16 bits motorola order (always) for jpeg header stuff.
//--------------------------------------------------------------------------
int Get16m(const void * Short)
{
    return (((uchar *)Short)[0] << 8) | ((uchar *)Short)[1];
}


//--------------------------------------------------------------------------
// Process a EXIF marker
// Describes all the drivel that most digital cameras include...
//--------------------------------------------------------------------------
void process_EXIF(unsigned char * CharBuf, unsigned int length)
{
    ImageInfo.FlashUsed = 0; // If it s from a digicam, and it used flash, it says so.

    FocalplaneXRes = 0;
    FocalplaneUnits = 0;
    ExifImageWidth = 0;

    if (ShowTags)
    {
        printf("Exif header %d bytes long\n",length);
    }

    {   // Check the EXIF header component
        static const uchar ExifHeader[] = "Exif\0\0";
        if (memcmp(CharBuf+2, ExifHeader,6)){
            ErrExit("Incorrect Exif header");
	    return;
        }
    }

    if (memcmp(CharBuf+8,"II",2) == 0){
        if (ShowTags) printf("Exif section in Intel order\n");
        MotorolaOrder = 0;
    }else{
        if (memcmp(CharBuf+8,"MM",2) == 0){
            if (ShowTags) printf("Exif section in Motorola order\n");
            MotorolaOrder = 1;
        }else{
            ErrExit("Invalid Exif alignment marker.");
	    return;
        }
    }

    // Check the next two values for correctness.
    if (Get16u(CharBuf+10) != 0x2a
      || Get32u(CharBuf+12) != 0x08){
        ErrExit("Invalid Exif start (1)");
	return;
    }

    LastExifRefd = CharBuf;

    // First directory starts 16 bytes in.  Offsets start at 8 bytes in.
    ProcessExifDir(CharBuf+16, CharBuf+8, length-6);

    // This is how far the interesting (non thumbnail) part of the exif went.
    ExifSettingsLength = LastExifRefd - CharBuf;

    // Compute the CCD width, in milimeters.
    if (FocalplaneXRes != 0){
        ImageInfo.CCDWidth = (float)(ExifImageWidth * FocalplaneUnits / FocalplaneXRes);
    }

    if (ShowTags){
        printf("Non settings part of Exif header: %d bytes\n",length-ExifSettingsLength);
    }
}

//--------------------------------------------------------------------------
// Figure out how much of the exif to keep
//--------------------------------------------------------------------------
int GetExifNonThumbnailSize(void)
{
    // This function must be called after ProcessExif has been called!
    return ExifSettingsLength;
}


//--------------------------------------------------------------------------
// Convert exif time to Unix time structure
//--------------------------------------------------------------------------
int Exif2tm(struct tm * timeptr, char * ExifTime)
{
    int a;

    timeptr->tm_wday = -1;

    // Check for format: YYYY:MM:DD HH:MM:SS format.
    a = sscanf(ExifTime, "%d:%d:%d %d:%d:%d",
            &timeptr->tm_year, &timeptr->tm_mon, &timeptr->tm_mday,
            &timeptr->tm_hour, &timeptr->tm_min, &timeptr->tm_sec);
        
    if (a == 6){
        timeptr->tm_isdst = -1;  
        timeptr->tm_mon -= 1;      // Adjust for unix zero-based months 
        timeptr->tm_year -= 1900;  // Adjust for year starting at 1900 
        return TRUE; // worked. 
    }

    return FALSE; // Wasn't in Exif date format.
}

//--------------------------------------------------------------------------
// Show the collected image info, displaying camera F-stop and shutter speed
// in a consistent and legible fashion.
//--------------------------------------------------------------------------
QString ShowImageInfo(void)
{
    int a;
    QString tag;
    //printf("File name    : %s\n",ImageInfo.FileName);
    tag+=QString().sprintf("File name    : %s\n",ImageInfo.FileName);
    //printf("File size    : %d bytes\n",ImageInfo.FileSize);
    tag+=QString().sprintf("File size    : %d bytes\n",ImageInfo.FileSize);

    {
        char Temp[20];
        struct tm ts;
        ts = *localtime(&ImageInfo.FileDateTime);
        strftime(Temp, 20, "%Y:%m:%d %H:%M:%S", &ts);
        //printf("File date    : %s\n",Temp);
	tag+=QString().sprintf("File date    : %s\n",Temp);
    }

    if (ImageInfo.CameraMake[0]){
        //printf("Camera make  : %s\n",ImageInfo.CameraMake);
	tag+=QString().sprintf("Camera make  : %s\n",ImageInfo.CameraMake);
        //printf("Camera model : %s\n",ImageInfo.CameraModel);
	tag+=QString().sprintf("Camera model : %s\n",ImageInfo.CameraModel);
    }
    if (ImageInfo.DateTime[0]){
        //printf("Date/Time    : %s\n",ImageInfo.DateTime);
	tag+=QString().sprintf("Date/Time    : %s\n",ImageInfo.DateTime);
    }
    //printf("Resolution   : %d x %d\n",ImageInfo.Width, ImageInfo.Height);
    tag+=QString().sprintf("Resolution   : %d x %d\n",ImageInfo.Width, ImageInfo.Height);
    if (ImageInfo.IsColor == 0){
        //printf("Color/bw     : Black and white\n");
	tag+=QString().sprintf("Color/bw     : Black and white\n");
    }
    if (ImageInfo.FlashUsed >= 0){
        //printf("Flash used   : %s\n",ImageInfo.FlashUsed ? "Yes" :"No");
	tag+=QString().sprintf("Flash used   : %s\n",ImageInfo.FlashUsed ? "Yes" :"No");
    }
    if (ImageInfo.FocalLength){
        //printf("Focal length : %4.1fmm",(double)ImageInfo.FocalLength);
        tag+=QString().sprintf("Focal length : %4.1fmm",(double)ImageInfo.FocalLength);
	if (ImageInfo.CCDWidth){
            //printf("  (35mm equivalent: %dmm)",
            //            (int)(ImageInfo.FocalLength/ImageInfo.CCDWidth*35 + 0.5));
	    tag+=QString().sprintf("  (35mm equivalent: %dmm)",
                        (int)(ImageInfo.FocalLength/ImageInfo.CCDWidth*35 + 0.5));
        }
        //printf("\n");
	tag+=QString().sprintf("\n");
    }

    if (ImageInfo.CCDWidth){
        //printf("CCD Width    : %4.2fmm\n",(double)ImageInfo.CCDWidth);
	tag+=QString().sprintf("CCD Width    : %4.2fmm\n",(double)ImageInfo.CCDWidth);
    }

    if (ImageInfo.ExposureTime){
        //printf("Exposure time:%6.3f s ",(double)ImageInfo.ExposureTime);
	tag+=QString().sprintf("Exposure time:%6.3f s ",(double)ImageInfo.ExposureTime);
        if (ImageInfo.ExposureTime <= 0.5){
            //printf(" (1/%d)",(int)(0.5 + 1/ImageInfo.ExposureTime));
	    tag+=QString().sprintf(" (1/%d)",(int)(0.5 + 1/ImageInfo.ExposureTime));
        }
        //printf("\n");
	tag+=QString().sprintf("\n");
    }
    if (ImageInfo.ApertureFNumber){
        //printf("Aperture     : f/%3.1f\n",(double)ImageInfo.ApertureFNumber);
	tag+=QString().sprintf("Aperture     : f/%3.1f\n",(double)ImageInfo.ApertureFNumber);
    }
    if (ImageInfo.Distance){
        if (ImageInfo.Distance < 0){
            //printf("Focus Dist.  : Infinite\n");
	    tag+=QString().sprintf("Focus Dist.  : Infinite\n");
        }else{
            //printf("Focus Dist.  :%5.2fm\n",(double)ImageInfo.Distance);
	    tag+=QString().sprintf("Focus Dist.  :%5.2fm\n",(double)ImageInfo.Distance);
        }
    }





    if (ImageInfo.ISOequivalent){ // 05-jan-2001 vcs
        //printf("ISO equiv.   : %2d\n",(int)ImageInfo.ISOequivalent);
	tag+=QString().sprintf("ISO equiv.   : %2d\n",(int)ImageInfo.ISOequivalent);
    }
    if (ImageInfo.ExposureBias){ // 05-jan-2001 vcs
        //printf("Exposure bias:%4.2f\n",(double)ImageInfo.ExposureBias);
	tag+=QString().sprintf("Exposure bias:%4.2f\n",(double)ImageInfo.ExposureBias);
    }
        
    if (ImageInfo.Whitebalance){ // 05-jan-2001 vcs
        switch(ImageInfo.Whitebalance) {
        case 1:
            //printf("Whitebalance : sunny\n");
	    tag+=QString().sprintf("Whitebalance : sunny\n");
            break;
        case 2:
            //printf("Whitebalance : fluorescent\n");
	    tag+=QString().sprintf("Whitebalance : fluorescent\n");
            break;
        case 3:
            //printf("Whitebalance : incandescent\n");
	    tag+=QString().sprintf("Whitebalance : incandescent\n");
            break;
        default:
            //printf("Whitebalance : cloudy\n");
	    tag+=QString().sprintf("Whitebalance : cloudy\n");
        }
    }
    if (ImageInfo.MeteringMode){ // 05-jan-2001 vcs
        switch(ImageInfo.MeteringMode) {
        case 2:
            //printf("Metering Mode: center weight\n");
	    tag+=QString().sprintf("Metering Mode: center weight\n");
            break;
        case 3:
            //printf("Metering Mode: spot\n");
	    tag+=QString().sprintf("Metering Mode: spot\n");
            break;
        case 5:
            //printf("Metering Mode: matrix\n");
	    tag+=QString().sprintf("Metering Mode: matrix\n");
            break;
        }
    }
    if (ImageInfo.ExposureProgram){ // 05-jan-2001 vcs
        switch(ImageInfo.ExposureProgram) {
        case 2:
            //printf("Exposure     : program (auto)\n");
	    tag+=QString().sprintf("Exposure     : program (auto)\n");
            break;
        case 3:
            //printf("Exposure     : aperture priority (semi-auto)\n");
	    tag+=QString().sprintf("Exposure     : aperture priority (semi-auto)\n");
            break;
        case 4:
            //printf("Exposure     : shutter priority (semi-auto)\n");
	    tag+=QString().sprintf("Exposure     : shutter priority (semi-auto)\n");
            break;
        }
    }
    if (ImageInfo.CompressionLevel){ // 05-jan-2001 vcs
        switch(ImageInfo.CompressionLevel) {
        case 1:
            //printf("JPG Quality  : basic\n");
	    tag+=QString().sprintf("JPG Quality  : basic\n");
            break;
        case 2:
            //printf("JPG Quality  : normal\n");
	    tag+=QString().sprintf("JPG Quality  : normal\n");
            break;
        case 4:
            //printf("JPG Quality  : fine\n");
	    tag+=QString().sprintf("JPG Quality  : fine\n");
            break;
       }
    }

         

    for (a=0;;a++){
        if (ProcessTable[a].Tag == ImageInfo.Process || ProcessTable[a].Tag == 0){
            //printf("Jpeg process : %s\n",ProcessTable[a].Desc);
	    tag+=QString().sprintf("Jpeg process : %s\n",ProcessTable[a].Desc);
            break;
        }
    }



    // Print the comment. Print 'Comment:' for each new line of comment.
    if (ImageInfo.Comments[0]){
        int a,c;
        //printf("Comment      : ");
	tag+=QString().sprintf("Comment      : ");
        for (a=0;a<MAX_COMMENT;a++){
            c = ImageInfo.Comments[a];
            if (c == '\0') break;
            if (c == '\n'){
                //printf("\nComment      : ");
		tag+=QString().sprintf("\nComment      : ");
            }else{
                //putchar(c);
		tag+=QString().sprintf("%c",c);
            }
        }
        //printf("\n");
	tag+=QString().sprintf("\n");
    }

    //printf("\n");
    tag+=QString().sprintf("\n");
    
    return tag;
}


//--------------------------------------------------------------------------
// Summarize highlights of image info on one line (suitable for grep-ing)
//--------------------------------------------------------------------------
void ShowConciseImageInfo(void)
{
    printf("\"%s\"",ImageInfo.FileName);

    printf(" %dx%d",ImageInfo.Width, ImageInfo.Height);

    if (ImageInfo.ExposureTime){
        printf(" (1/%d)",(int)(0.5 + 1/ImageInfo.ExposureTime));
    }

    if (ImageInfo.ApertureFNumber){
        printf(" f/%3.1f",(double)ImageInfo.ApertureFNumber);
    }

    if (ImageInfo.FocalLength){
        if (ImageInfo.CCDWidth){
            // 35 mm equivalent focal length.
            printf(" f(35)=%dmm",(int)(ImageInfo.FocalLength/ImageInfo.CCDWidth*35 + 0.5));
        }
    }

    if (ImageInfo.FlashUsed > 0){
        printf(" (flash)");
    }

    if (ImageInfo.IsColor == 0){
        printf(" (bw)");
    }

    printf("\n");
}
//--------------------------------------------------------------------------
// Do selected operations to one file at a time.
//--------------------------------------------------------------------------
QString ProcessFile(const char * FileName, bool showsize, const char *ThumbnailName)
{
    QString ret;
    
    int Modified = FALSE;
    ReadMode_t ReadMode = READ_EXIF;
    
    
    CurrentFile = FileName;

    // Start with an empty image information structure.
    memset(&ImageInfo, 0, sizeof(ImageInfo));
    memset(&Sections, 0, sizeof(Sections));

    ImageInfo.FlashUsed = -1;
    ImageInfo.MeteringMode = -1;

    // Store file date/time.
    {
        struct stat st;
        if (stat(FileName, &st) >= 0){
            ImageInfo.FileDateTime = st.st_mtime;
            ImageInfo.FileSize = st.st_size;
        }else{
            ErrExit("No such file");
	    return 0;
        }
    }

    strncpy(ImageInfo.FileName, FileName, _MAX_PATH);



   if (ExifSourceName){
        char RelativeExifName[_MAX_PATH+1];

        // Make a relative name.
        RelativeName(RelativeExifName, ExifSourceName, FileName);

        if(!ReadJpegFile(RelativeExifName, READ_EXIF)) return QString();

        DiscardAllButExif();    // Don't re-read exif section again on next read.

        Modified = TRUE;
        ReadMode = READ_IMAGE;
    }

    FilesMatched += 1;

    FilesMatched = TRUE; // Turns off complaining that nothing matched.

    if (TrimExif || DeleteComments){
        ReadMode = (ReadMode_t)(ReadMode|READ_IMAGE);
    }

    if (!ReadJpegFile(FileName, ReadMode)) return QString();
    
    if(showsize)
    {
    	ret= QString().sprintf("%dx%d", ImageInfo.Width, ImageInfo.Height);
	DiscardData();
	return ret;
    }


    if (CheckFileSkip()){
        DiscardData();
        return QString();
    }

    if (ShowConcise){
        ShowConciseImageInfo();
    }else{
        if (!DoAction || ShowTags){
            ret = ShowImageInfo();
        }
    }

    if (ThumbnailName){
        if (ImageInfo.ThumbnailPointer){
            FILE * ThumbnailFile;
            char OutFileName[_MAX_PATH+1];

            // Make a relative name.
            RelativeName(OutFileName, ThumbnailName, FileName);

            ThumbnailFile = fopen(OutFileName,"wb");
            if (ThumbnailFile)
	    {
                fwrite(ImageInfo.ThumbnailPointer, ImageInfo.ThumbnailSize ,1, ThumbnailFile);
                fclose(ThumbnailFile);
                //fprintf(stderr, "EXIF: Created: '%s'\n", OutFileName);
		ret="OK";
            }
	    else
	    {
                ErrExit("Could not write thumbnail file: ");
                ErrExit(OutFileName);
		return 0;
            }
        }else
	{
            //fprintf(stderr, "EXIF: Image '%s' contains no thumbnail\n",FileName);
	    ret="ERROR";
        }
    }

    if (TrimExif){
        if (TrimExifFunc()) Modified = TRUE;
    }
    
    if (DeleteComments){
        if (RemoveComments()) Modified = TRUE;
    }


    if (Modified){
        char BackupName[400];
        printf("Modified: %s\n",FileName);

        strcpy(BackupName, FileName);
        strcat(BackupName, ".t");

        // Remove any .old file name that may pre-exist
        unlink(BackupName);

        // Rename the old file.
        rename(FileName, BackupName);

        // Write the new file.
        WriteJpegFile(FileName);

        // Now that we are done, remove original file.
        unlink(BackupName);
    }


    if(0){
        printf("Error: Time '%s': cannot convert to Unix time\n",ImageInfo.DateTime);
    }
    DiscardData();
    
    return ret;
}
