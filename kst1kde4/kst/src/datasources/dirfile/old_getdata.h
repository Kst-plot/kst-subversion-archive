
#ifndef GETDATA_H
#define GETDATA_H

extern const char *GD_ERROR_CODES[23];

#define GD_E_OK                0
#define GD_E_OPEN_FORMAT       1
#define GD_E_FORMAT            2
#define GD_E_FIELD             4
#define GD_E_BAD_CODE          5
#define GD_E_BAD_RETURN_TYPE   6
#define GD_E_OPEN_RAWFIELD     7
#define GD_E_OPEN_INCLUDE      8
#define GD_E_INTERNAL_ERROR    9
#define GD_E_NO_RAW_FIELDS    10
#define GD_E_ALLOC            11
#define GD_E_SIZE_MISMATCH    12
#define GD_E_OPEN_LINFILE     13
#define GD_E_RECURSE_LEVEL    14

#define PD_E_BAD_CODE         15
#define PD_E_OPEN_RAWFIELD    16
#define PD_E_CLOSE_RDONLY     17
#define PD_E_WRITE_LOCK       18
#define PD_E_FLOCK_ALLOC      19
#define PD_E_MULT_LINCOM      20
#define ENDIAN_ERROR          21


/***************************************************************************/
/*                                                                         */
/*  GetData: read BLAST format files.                                      */
/*    filename_in: the name of the file directory (raw files are in here)  */
/*    field_code: the name of the field you want to read                   */
/*    first_frame, first_samp: the first sample read is                    */
/*              first_samp + samples_per_frame*first_frame                 */
/*    num_frames, num_samps: the number of samples read is                 */
/*              num_samps + samples_per_frame*num_frames                   */
/*    return_type: data type of *data_out.  's': 16 bit signed             */
/*              'u' 16bit unsiged. 'S' 32bit signed 'U' 32bit unsigned     */
/*              'c' 8 bit unsigned                                         */
/*    void *data_out: array to put the data                                */
/*    *error_code: error code is returned here. If error_code==null,       */
/*               GetData prints the error message and exits                */
/*                                                                         */
/*    return value: returns number of samples actually read into data_out  */
/*                                                                         */
/***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
int GetData(const char *filename_in, const char *field_code,
             int first_sframe, int first_samp,
             int num_sframes, int num_samp,
             char return_type, void *data_out,
            int *error_code);

/***************************************************************************/
/*                                                                         */
/*    GetDataErrorString: Write a descriptive message in the supplied      */
/*    buffer describing the last library error.  The message may be        */
/*    truncated but should be null terminated.                             */
/*                                                                         */
/*      buffer: memory into which to write the string                      */
/*      buflen: length of the buffer.  GetDataErrorString will not write   */
/*              more than buflen characters (including the trailing '\0')  */
/*                                                                         */
/*    return value: buffer or NULL if buflen < 1                           */
/*                                                                         */
/***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
char* GetDataErrorString(char* buffer, size_t buflen);

/***************************************************************************/
/*                                                                         */
/*    Get the number of samples for each frame for the given field         */
/*                                                                         */
/***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
int GetSamplesPerFrame(const char *filename_in, const char *field_name, int *error_code);

/***************************************************************************/
/*                                                                         */
/*    Get the number of frames available                                   */
/*                                                                         */
/***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
int GetNFrames(const char *filename_in, int *error_code, const char *field);

/***************************************************************************/
/*                                                                         */
/*    Get the number format structure                                      */
/*                                                                         */
/***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
struct FormatType *GetFormat(const char *filedir, int *error_code);

/***************************************************************************/
/*                                                                         */
/*  PutData: write dirfile format files.                                   */
/*    filename_in: the name of the file directory (raw files are in here)  */
/*    field_code: the name of the field you want to write                  */
/*    first_frame, first_samp: the first sample written is                 */
/*              first_samp + samples_per_frame*first_frame                 */
/*    num_frames, num_samps: the number of samples written is              */
/*              num_samps + samples_per_frame*num_frames                   */
/*    data_type: data type of *data_in.  's': 16 bit signed                */
/*              'u' 16bit unsigned 'S' 32bit signed 'U' 32bit unsigned     */
/*              'c' 8 bit unsigned 'f' 32bit float 'd' 64bit double        */
/*    void *data_in: array containing the data                             */
/*    *error_code: error code is returned here. If error_code==null,       */
/*               PutData prints the error message and exits                */
/*                                                                         */
/*    return value: returns # of samples actually written to file          */
/*                                                                         */
/***************************************************************************/

#ifdef __cplusplus
extern "C"
#endif
int PutData(const char *filename_in, const char *field_code,
      int first_frame, int first_samp,
      int num_frames, int num_samp,
      char data_type, void *data_in,
      int *error_code);
#endif
