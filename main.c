
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <proj_api.h>

#include "nvutility.h"

#include "pfm.h"
#include "pfm_extras.h"

#include "llz.h"

#include "version.h"


/*****************************************************************************

    Program:    pfm2rdp

    Purpose:    Converts PFM bin or index data to rdp format for input to the
                chrtr gridding program.

    Programmer: Jan C. Depner

    Date:       11/08/99

*****************************************************************************/


void usage ()
{
  fprintf (stderr, 
           "\nUsage: pfm2rdp OUTPUT_BASE_FILENAME [-M -X -V -A -a -r AREA_FILE -c -i -m MAX_FILE_SIZE -u -f] <PFM_HANDLE_FILE or PFM_LIST_FILE>\n");
  fprintf (stderr, "\nWhere:\n\n");
  fprintf (stderr, "\t-M, -X, and -A are mutually exclusive:\n\n");
  fprintf (stderr, "\t-M  =  use the minimum filtered surface (default)\n");
  fprintf (stderr, "\t-X  =  use the maximum filtered surface\n");
  fprintf (stderr, "\t-V  =  use the average filtered surface\n");
  fprintf (stderr, "\t-A  =  use all soundings not just the bin surfaces\n");
  fprintf (stderr, "\t-a  =  output in ASCII YXZ format\n");
  fprintf (stderr, "\t-l  =  output in LLZ format\n");
  fprintf (stderr, "\t-c  =  only output data from checked or verified bins\n");
  fprintf (stderr, "\t-i  =  include valid reference data\n");
  fprintf (stderr, "\t-m  =  files will be broken up into MAX_FILE_SIZE byte\n");
  fprintf (stderr, "\t\tsized files.  Default is 200000000 bytes.\n");
  fprintf (stderr, "\t-r  =  generate data only for a defined area.\n");
  fprintf (stderr, "\t-u  =  convert output positions to UTM northing, easting, and zone (only valid for ASCII output)\n");
  fprintf (stderr, "\t-f  =  flip sign on Z value (change depth to elevation or elevation to depth).\n");
  fprintf (stderr, "\t\tAREA_FILE = Area file name (generate data only for defined area).\n");
  fprintf (stderr, "\t\tThe area file name must have a .ARE extension\n");
  fprintf (stderr, "\t\tfor ISS60 type area files.  All others are assumed to be\n");
  fprintf (stderr, "\t\tgeneric area files which consist of a simple list of\n");
  fprintf (stderr, "\t\tpolygon points.  The points may be in any of the following\n");
  fprintf (stderr, "\t\tformats:\n\n");
  fprintf (stderr, "\t\t\tHemisphere Degrees Minutes Seconds.decimal\n");
  fprintf (stderr, "\t\t\tHemisphere Degrees Minutes.decimal\n");
  fprintf (stderr, "\t\t\tHemisphere Degrees.decimal\n");
  fprintf (stderr, "\t\t\tSign Degrees Minutes Seconds.decimal\n");
  fprintf (stderr, "\t\t\tSign Degrees Minutes.decimal\n");
  fprintf (stderr, "\t\t\tSign Degrees.decimal\n\n");
  fprintf (stderr, "\t\tThe lat and lon must be entered one per line, separated by\n");
  fprintf (stderr, "\t\ta comma.  You do not need to repeat the first point, the\n");
  fprintf (stderr, "\t\tpolygon will be closed automatically.\n\n");
  fprintf (stderr, "IMPORTANT NOTE: This program is deprecated.  Please use pfmExtract instead!\n\n");
  fflush (stderr);
}



int32_t main (int32_t argc, char **argv)
{
  FILE                *fp = NULL;
  int32_t             i, j, k, percent = 0, old_percent = -1, endian, recnum, rdp_data[3],
                      pfm_handle, out_count = 0, file_count = 0, type = 0, polygon_count = 0,
                      x_start, y_start, width, height, llz_hnd = -1, size = 0, max_file_size = 2000000000;
  float               total;
  BIN_RECORD          bin;
  DEPTH_RECORD        *depth;
  char                out_file[256], orig[256], areafile[512];
  NV_I32_COORD2       coord;
  uint8_t             ascii = NVFalse, llz = NVFalse, checked = NVFalse, reference = NVFalse, utm = NVFalse, 
                      flip = NVFalse;
  PFM_OPEN_ARGS       open_args;
  char                c;
  double              polygon_x[200], polygon_y[200], central_meridian;
  NV_F64_XYMBR        mbr;
  int32_t             zone = 0;
  double              x, y;
  LLZ_REC             llz_rec;
  LLZ_HEADER          llz_header;
  projPJ              pj_utm = NULL, pj_latlon = NULL;
  extern char         *optarg;
  extern int          optind;



  printf ("\n\n %s \n\n\n", VERSION);

  printf ("IMPORTANT NOTE: This program is deprecated.  Please use pfmExtract instead!\n\n");


  type = MIN_FILTERED_DEPTH;
  ascii = NVFalse;
  llz = NVFalse;
  flip = NVFalse;
  checked = NVFalse;
  reference = NVFalse;
  max_file_size = 2000000000;
  utm = NVFalse;

  while ((c = getopt (argc, argv, "MXVAalcir:m:uf")) != EOF)
    {
      switch (c)
        {
        case 'M':
          type = MIN_FILTERED_DEPTH;
          break;

        case 'X':
          type = MAX_FILTERED_DEPTH;
          break;

        case 'V':
          type = AVERAGE_FILTERED_DEPTH;
          break;

        case 'A':
          type = 999;
          break;

        case 'a':
          ascii = NVTrue;
          break;

        case 'l':
          llz = NVTrue;
          break;

        case 'c':
          checked = NVTrue;
          break;

        case 'i':
          reference = NVTrue;
          break;

        case 'm':
          sscanf (optarg, "%d", &max_file_size);
          break;

        case 'r':
          strcpy (areafile, optarg);
          get_area_mbr (areafile, &polygon_count, polygon_x, polygon_y, &mbr);
          break;

        case 'u':
          utm = NVTrue;
          break;

        case 'f':
          flip = NVTrue;
          break;

        default:
          usage ();
          exit (-1);
          break;
        }
    }

  if (optind >= argc)
    {
      usage ();
      exit (-1);
    }

  strcpy (orig, argv[optind + 1]);

  if (ascii)
    {
      sprintf (out_file, "%s.%02d.txt", orig, file_count);
    }
  else if (llz)
    {
      sprintf (out_file, "%s.%02d.llz", orig, file_count);
    }
  else
    {
      sprintf (out_file, "%s.%02d.rdp", orig, file_count);
    }

  if (llz)
    {
      /*  Boilerplate LLZ header.  */

      sprintf (llz_header.comments, "Created from %s using %s", pfm_basename (argv[optind]), VERSION);
      llz_header.time_flag = NVFalse;
      llz_header.depth_units = 0;

      if ((llz_hnd = create_llz (out_file, llz_header)) < 0)
	{
	  perror (out_file);
	  exit (-1);
	}	  
    }
  else
    {
      if ((fp = fopen (out_file, "w")) == NULL)
	{
	  perror (out_file);
	  exit (-1);
	}
    }

  strcpy (open_args.list_path, argv[optind]);

  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);

  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  if (open_args.head.proj_data.projection)
    {
      fprintf (stderr, "Sorry, this program does not work with projected PFM structures.\n");
      exit (-1);
    }


  /*  If we're doing UTM output, set the projection and get the zone.  */

  if (utm)
    {
      central_meridian = open_args.head.mbr.min_x + (open_args.head.mbr.max_x - open_args.head.mbr.min_x) / 2.0;

      zone = (int32_t) (31.0 + central_meridian / 6.0);
      if (zone >= 61) zone = 60;	
      if (zone <= 0) zone = 1;

      char string[128];
      if (open_args.head.mbr.max_y < 0.0)
        {
          sprintf (string, "+proj=utm +zone=%d +lon_0=%.9f +ellps=WGS84 +datum=WGS84 +south", zone, central_meridian);
        }
      else
        {
          sprintf (string, "+proj=utm +zone=%d +lon_0=%.9f +ellps=WGS84 +datum=WGS84", zone, central_meridian);
        }

      if (!(pj_utm = pj_init_plus (string)))
        {
          fprintf (stderr, "Error initializing UTM projection\n");
          exit (-1);
        }

      if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
        {
          fprintf (stderr, "Error initializing latlon projection\n");
          exit (-1);
        }
    }


  fprintf (stderr, "\n\nFILE : %s\n\n", out_file);
  fflush (stderr);


  endian = 0x00010203;
  if (!ascii && !llz) fwrite (&endian, sizeof (int32_t), 1, fp);


  /*  If we're doing this by area there is no need to go through
      the entire file so we'll generate starts and ends based on
      the mbr.  */

  x_start = 0;
  y_start = 0;
  width = open_args.head.bin_width;
  height = open_args.head.bin_height;

  if (polygon_count)
    {
      if (mbr.min_y > open_args.head.mbr.max_y || mbr.max_y < open_args.head.mbr.min_y ||
          mbr.min_x > open_args.head.mbr.max_x || mbr.max_x < open_args.head.mbr.min_x)
        {
          fprintf (stderr, "\n\nSpecified area is completely outside of the PFM bounds!\n\n");
          exit (-1);
        }


      /*  Match to nearest cell.  */

      x_start = NINT ((mbr.min_x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees);
      y_start = NINT ((mbr.min_y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees);
      width = NINT ((mbr.max_x - mbr.min_x) / open_args.head.x_bin_size_degrees);
      height = NINT ((mbr.max_y - mbr.min_y) / open_args.head.y_bin_size_degrees);


      /*  Adjust to PFM bounds if necessary.  */

      if (x_start < 0) x_start = 0;
      if (y_start < 0) y_start = 0;
      if (x_start + width > open_args.head.bin_width) width = open_args.head.bin_width - x_start;
      if (y_start + height > open_args.head.bin_height) height = open_args.head.bin_height - y_start;


      /*  Redefine bounds.  */

      mbr.min_x = open_args.head.mbr.min_x + x_start * open_args.head.x_bin_size_degrees;
      mbr.min_y = open_args.head.mbr.min_y + y_start * open_args.head.y_bin_size_degrees;
      mbr.max_x = mbr.min_x + width * open_args.head.x_bin_size_degrees;
      mbr.max_y = mbr.min_y + height * open_args.head.y_bin_size_degrees;
    }


  total = open_args.head.bin_height;


  /* double loop over height & width of bins or area */

  for (i = y_start ; i < y_start + height ; i++)
    {
      coord.y = i;
      for (j = x_start ; j < x_start + width ; j++)
        {
          coord.x = j;

          read_bin_record_index (pfm_handle, coord, &bin);

          if (!checked || (bin.validity & (PFM_CHECKED | PFM_VERIFIED)))
            {
              switch (type)
                {
                case 999:
                  if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                    {
                      for (k = 0 ; k < recnum ; k++)
                        {
                          if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED)))
                            {
                              if (!polygon_count || 
                                  inside (polygon_x, polygon_y, polygon_count, 
                                          depth[k].xyz.x, depth[k].xyz.y))
                                {
                                  if (!(depth[k].validity & PFM_REFERENCE) || reference)
                                    {
                                      out_count++;

                                      if (flip) depth[k].xyz.z = -depth[k].xyz.z;

                                      if (ascii)
                                        {
                                          if (utm)
                                            {
                                              x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                              y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                              pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                              fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, depth[k].xyz.z);
                                            }
                                          else
                                            {
                                              fprintf (fp, "%.9f,%.9f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, 
                                                       depth[k].xyz.z);
                                            }
                                        }
				      else if (llz)
					{
					  llz_rec.xy.lat = depth[k].xyz.y;
					  llz_rec.xy.lon = depth[k].xyz.x;
					  llz_rec.depth = depth[k].xyz.z;
					  llz_rec.status = 0;

					  append_llz (llz_hnd, llz_rec);
					}
                                      else
                                        {
                                          rdp_data[0] = depth[k].xyz.y * 10000000.0;
                                          rdp_data[1] = depth[k].xyz.x * 10000000.0;
                                          rdp_data[2] = depth[k].xyz.z * 10000.0;

                                          fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                                        }
                                    }
                                }
                            }
                        }

                      free (depth);
                    }
                  break;

                case MIN_FILTERED_DEPTH:
                case MAX_FILTERED_DEPTH:

                  if (bin.validity & PFM_DATA)
                    {
                      if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                        {
                          for (k = 0 ; k < recnum ; k++)
                            {
                              if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED)))
                                {
                                  if (type == MIN_FILTERED_DEPTH)
                                    {
                                      if (fabs ((double)
                                                (bin.min_filtered_depth -
                                                 depth[k].xyz.z)) < 0.0005)
                                        {
                                          if (flip) depth[k].xyz.z = -depth[k].xyz.z;

                                          if (ascii)
                                            {
                                              if (utm)
                                                {
                                                  x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                                  y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                                  fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, depth[k].xyz.z);
                                                }
                                              else
                                                {
                                                  fprintf (fp, "%.9f,%.9f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x,
                                                           depth[k].xyz.z);
                                                }
                                            }
					  else if (llz)
					    {
					      llz_rec.xy.lat = depth[k].xyz.y;
					      llz_rec.xy.lon = depth[k].xyz.x;
					      llz_rec.depth = depth[k].xyz.z;
					      llz_rec.status = 0;

					      append_llz (llz_hnd, llz_rec);
					    }
                                          else
                                            {
                                              rdp_data[0] = depth[k].xyz.y * 10000000.0;
                                              rdp_data[1] = depth[k].xyz.x * 10000000.0;
                                              rdp_data[2] = depth[k].xyz.z * 10000.0;

                                              fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                                            }
                                          out_count++;
                                          break;
                                        }
                                    }
                                  else
                                    {
                                      if (fabs ((double) (bin.max_filtered_depth - depth[k].xyz.z)) < 0.005)
                                        {
                                          if (flip) depth[k].xyz.z = -depth[k].xyz.z;

                                          if (ascii)
                                            {
                                              if (utm)
                                                {
                                                  x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                                  y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                                  fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, depth[k].xyz.z);
                                                }
                                              else
                                                {
                                                  fprintf (fp, "%.9f,%.9f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x,
                                                           depth[k].xyz.z);
                                                }
                                            }
					  else if (llz)
					    {
					      llz_rec.xy.lat = depth[k].xyz.y;
					      llz_rec.xy.lon = depth[k].xyz.x;
					      llz_rec.depth = depth[k].xyz.z;
					      llz_rec.status = 0;

					      append_llz (llz_hnd, llz_rec);
					    }
                                          else
                                            {
                                              rdp_data[0] = depth[k].xyz.y * 10000000.0;
                                              rdp_data[1] = depth[k].xyz.x * 10000000.0;
                                              rdp_data[2] = depth[k].xyz.z * 10000.0;

                                              fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                                            }
                                          out_count++;
                                          break;
                                        }
                                    }
                                }
                            }
                        }
                      free (depth);
                    }
                  break;


                case AVERAGE_FILTERED_DEPTH:

                  if (bin.validity & (PFM_DATA | PFM_INTERPOLATED))
                    {
                      if (flip) bin.avg_filtered_depth = -bin.avg_filtered_depth;

                      if (ascii)
                        {
                          if (utm)
                            {
                              x = bin.xy.x * NV_DEG_TO_RAD;
                              y = bin.xy.y * NV_DEG_TO_RAD;
                              pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                              fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, bin.avg_filtered_depth);
                            }
                          else
                            {
                              fprintf (fp, "%.9f,%.9f,%.2f\n", bin.xy.y, bin.xy.x, bin.avg_filtered_depth);
                            }
                        }
		      else if (llz)
			{
			  llz_rec.xy.lat = bin.xy.y;
			  llz_rec.xy.lon = bin.xy.x;
			  llz_rec.depth = bin.avg_filtered_depth;
			  llz_rec.status = 0;

			  append_llz (llz_hnd, llz_rec);
			}
                      else
                        {
                          rdp_data[0] = bin.xy.y * 10000000.0;
                          rdp_data[1] = bin.xy.x * 10000000.0;
                          rdp_data[2] = bin.avg_filtered_depth * 10000.0;

                          fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                        }
                      out_count++;
                    }
                  break;
                }
            }

          /*  Make sure file size does not exceed 2GB.  */

          if (llz)
	    {
	      size = ftell_llz (llz_hnd);
	    }
	  else
	    {
	      size = ftell (fp);
	    }

	  if (size >= max_file_size)
            {
              file_count++;


              if (ascii)
                {
		  fclose (fp);
                  sprintf (out_file, "%s.%02d.txt", orig, file_count);
                }
	      else if (llz)
		{
		  close_llz (llz_hnd);
                  sprintf (out_file, "%s.%02d.llz", orig, file_count);
		}
              else
                {
		  fclose (fp);
                  sprintf (out_file, "%s.%02d.rdp", orig, file_count);
                }


	      if (llz)
		{
		  /*  Boilerplate LLZ header.  */

		  sprintf (llz_header.comments, "Created from %s using %s", pfm_basename (open_args.list_path), VERSION);
		  llz_header.time_flag = NVFalse;
		  llz_header.depth_units = 0;

		  if ((llz_hnd = create_llz (out_file, llz_header)) < 0)
		    {
		      perror (out_file);
		      exit (-1);
		    }	  
		}
	      else
		{
		  if ((fp = fopen (out_file, "w")) == NULL)
		    {
		      perror (out_file);
		      exit (-1);
		    }
		}

              fprintf (stderr, "\n\nFILE : %s\n\n", out_file);
              fflush (stderr);

              if (!ascii && !llz) fwrite (&endian, sizeof (int32_t), 1, fp);
            }
        }



      percent = ((i - y_start) / total) * 100.0;
      if (percent != old_percent)
        {
          old_percent = percent;
          fprintf (stderr, "%03d%% processed            \r", percent);
          fflush (stderr);
        }
    }


  if (ascii)
    {
      fprintf (stderr, "%d ASCII records output        \n\n\n", out_count);
      fclose (fp);
    }
  else if (llz)
    {
      fprintf (stderr, "%d LLZ records output        \n\n\n", out_count);
      close_llz (llz_hnd);
    }
  else
    {
      fprintf (stderr, "%d RDP records output        \n\n\n", out_count);
      fclose (fp);
    }
  fflush (stderr);



  return (0);
}
