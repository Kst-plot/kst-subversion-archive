/*** Header file for readdata.c, which reads StratoPlate files ***/

#ifndef _READDATA_H
#define _READDATA_H
/* define error codes */
#define E_OK               0
#define E_OPEN_FFFILE      1
#define E_FFFILE_FORMAT    2
#define E_FILE_OPEN        3
#define E_BAD_FILETYPE     4
#define E_BAD_CODE         5
#define E_BAD_RETURN_TYPE  6

extern char *RD_ERROR_CODES[7];

extern "C" int ReadData(char *filename, const char *field_code,
             int first_sframe, int first_samp,
             int num_sframes, int num_samp,
             char return_type, void *data_out,
             int *error_code);
#endif
