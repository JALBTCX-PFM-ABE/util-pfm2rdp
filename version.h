
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/*********************************************************************************************

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#define     VERSION     "PFM Software - pfm2rdp V4.09 - 06/27/15"

#endif

/*

    Version 1.0
    Jan C. Depner
    09/04/00


    Version 1.1
    Jan C. Depner
    02/21/01

    Passing scale to open_pfm_file as a pointer.


    Version 2.0
    Jan C. Depner
    06/17/01

    Outputs min/avg/max instead of all data values.


    Version 2.1
    Jan C. Depner
    06/20/01

    Changed open_pfm_file to use structure args.


    Version 2.2
    Jan C. Depner
    07/19/01

    4.0 PFM library changes.


    Version 2.3
    Jan C. Depner
    10/03/01

    Moved file size check inside inner loop (just in case).  Changed output
    filenames to maintain constant extension.


    Version 2.31
    Jan C. Depner
    11/26/01

    Oops!  Fixed a test bug.


    Version 2.32
    Jan C. Depner
    05/02/02

    Set max file size to 650MB so it will fit on a CD.


    Version 2.33
    Jan C. Depner
    08/12/02

    Set max file size to 2000000000 so "compressed" it should fit on a CD.


    Version 3.0
    Jan C. Depner
    08/25/04

    Completely changed options to give more versatility.  Max file size is
    now an argument.  ASCII or RDP output.  Limit output to checked and/or
    verified bins.


    Version 3.2
    Jan C. Depner
    10/19/04

    Added ability to output data for defined areas instead of entire file.
    Fixed bug in "checked" option.  Fixed percent spinner.


    Version 3.3
    Jan C. Depner
    12/09/04

    Added option (-i) to output valid reference data.


    Version 3.31
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 3.32
    Jan C. Depner
    05/23/05

    Added .txt and .rdp file extensions to output files.


    Version 3.33
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 3.34
    Jan C. Depner
    06/05/06

    Removed inside.c and get_area_mbr.c.  Now in utility.


    Version 3.35
    Jan C. Depner
    06/28/06

    Fixed include reference option.


    Version 3.4
    Jan C. Depner
    07/31/06

    Added ASCII UTM output option.  Added flip sign option.


    Version 3.41
    Jan C. Depner
    08/31/07

    Get the UTM zone from the center of the PFM and don't recompute it for each point.


    Version 3.42
    Jan C. Depner
    10/22/07

    Added fflush calls after prints to stderr since flush is not automatic in Windows.


    Version 3.43
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 4.00
    Jan C. Depner
    06/12/08

    Added LLZ format output (soon to replace RDP).


    Version 4.01
    Jan C. Depner
    07/29/08

    Output interpolated data if dumping average surface (-V).


    Version 4.02
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 4.03
    Jan C. Depner
    06/08/09

    Added "deprecated - please use pfmExtract" message.


    Version 4.04
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 4.05
    Jan C. Depner
    05/06/11

    Fixed problem with getopt that only happens on Windows.


    Version 4.06
    Jan C. Depner (PFM Software)
    07/05/14

    - Had to change the argument order in pj_init_plus for the UTM projection.  Newer versions of 
      proj4 appear to be very sensitive to this.


    Version 4.07
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 4.08
    Jan C. Depner (PFM Software), Jim Hammack (New Wave Systems)
    04/03/15

    - Computes zone prior to defining UTM projection so that we don't need PROJ_LIB set (I think).


    Version 4.09
    Jan C. Depner (PFM Software)
    06/27/15

    - Fixed PROJ4 init problem.

*/
