
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
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>

#include "nvutility.h"

#include "shapefil.h"
#include "version.h"


/*

    Programmer : Jan C. Depner
    Date : 12/11/07

    Converts ISS60 area files (.ARE) or generic area files (.are) to
    ESRI polygon shape files.

*/



int32_t main (int32_t argc, char **argv)
{
  SHPHandle           shp_hnd;
  SHPObject           *shape;
  DBFHandle           dbf_hnd;
  FILE                *prj_fp;
  char                shape_name[512], prj_file[512];
  double              x[1000], y[1000], z[1000], m[1000];
  int32_t             i, polygon_count = 0;
  NV_F64_XYMBR        mbr;


  printf ("\n\n %s \n\n\n", VERSION);



  if (argc < 2)
    {
      fprintf (stderr, "\n\nUsage: %s INPUT_AREA_FILE\n", argv[0]);
      exit (-1);
    }


  if (strcmp (&argv[1][strlen (argv[1]) - 4], ".are") && strcmp (&argv[1][strlen (argv[1]) - 4], ".ARE"))
    {
      fprintf (stderr, "File extension MUST be .are or .ARE\n");
      exit (-1);
    }


  strcpy (shape_name, argv[1]);
  shape_name[strlen (shape_name) - 4] = 0;


  if ((shp_hnd = SHPCreate (shape_name, SHPT_POLYGON)) == NULL)
    {
      perror (shape_name);
      exit (-1);
    }


  if ((dbf_hnd = DBFCreate (shape_name)) == NULL)
    {
      perror (shape_name);
      exit (-1);
    }


  /*  Make attributes*/

  if (DBFAddField (dbf_hnd, "file", FTString, 15, 0 ) == -1)
    {
      perror ("Adding field to dbf file");
      exit (-1);
    }


  //  Stupid freaking .prj file

  strcpy (prj_file, shape_name);
  strcat (prj_file, ".prj");

  if ((prj_fp = fopen (prj_file, "w")) == NULL)
    {
      perror (prj_file);

      exit (-1);
    }

  fprintf (prj_fp, "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]\n");
  fclose (prj_fp);


  if (!get_area_mbr (argv[1], &polygon_count, x, y, &mbr))
    {
      fprintf (stderr, "\n\nError reading area file %s\n\n", argv[1]);
      exit (-1);
    }


  for (i = 0 ; i < polygon_count ; i++)
    {
      z[i] = 0.0;
      m[i] = 0.0;
    }


  shape = SHPCreateObject (SHPT_POLYGON, -1, 0, NULL, NULL, polygon_count, x, y, z, m);
  SHPWriteObject (shp_hnd, -1, shape);
  SHPDestroyObject (shape);
  DBFWriteStringAttribute (dbf_hnd, 0, 0, argv[1]);

  SHPClose (shp_hnd);
  DBFClose (dbf_hnd);  


  return (0);
}
