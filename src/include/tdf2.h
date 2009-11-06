C tdf2.h Parameters for TDF program, November 2004 version.

      INTEGER MIT,MTABC,MHEL,MBITS,MGAT,MGAR,MGAI,MALT,MHTS

C Allowed number of tabular entries in the TDF file. For large
C models there may be one item per zone so this should reflect
C the number of zones in building.h
      PARAMETER (MIT=72)

C Allowed columns of tabular data in TAB file.
      PARAMETER (MTABC=50)

C Maximum number of help lines associated with a generic item.
      PARAMETER (MHEL=5)

C Maximum number of non-static bits in a generic item period data set.
      PARAMETER (MBITS=18)

C Maximum number of text fields.
      PARAMETER (MGAT=12)

C Maximum number of real fields.
      PARAMETER (MGAR=18)

C Maximum number of integer fields.
      PARAMETER (MGAI=12)

C Maximum number of text alternatives in a text field.
      PARAMETER (MALT=6)

C Maximum number of timesteps in an hour.
      PARAMETER (MHTS=60)
