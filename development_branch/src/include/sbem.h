C NCM Array sizing integers
      integer MFT, MPT, MHT, MHC, MCS, MSS, MBT, MREG, MSTG, MSBT
      integer MDW, MWS, MNS, MAC, MACL, MILS
      PARAMETER (MFT=12)    ! max. fuel types
      PARAMETER (MPT=32)    ! max. system types
      PARAMETER (MHT=25)    ! max. HVAC system types
      PARAMETER (MHC=3)     ! max. system cooling types
      PARAMETER (MCS=MHC*8) ! max. cooling mechanism COP MHC*8
      PARAMETER (MSS=3)     ! max. building services strategies
      PARAMETER (MBT=29)    ! max. building types 
      PARAMETER (MREG=4)    ! max. building regulations to follow
      PARAMETER (MSTG=2)    ! max. building design stages
      PARAMETER (MSBT=3)    ! max. Scottish building types
      PARAMETER (MDW=5)     ! max. DHW generators
      PARAMETER (MWS=6)     ! max. fuel types for DHW generators
      
C WARNING: MNS should be at least as big as MCOM in building.h.
      PARAMETER (MNS=28)    ! max. HVAC and DHW systems allowed in model
                            ! max. activities allowed in the model
                            ! This is kept equal to max zones normally allowed
      PARAMETER (MAC=65)    ! max. activity types 
      PARAMETER (MACL=525)  ! max. activity types (global list) 
      PARAMETER (MILS=10)   ! max. lighting types

C NCM common blocks for reading SBEM database
C Integers
      COMMON/SBEM01/IBRUKLH(MFT,MPT,MHT),ISYSAPP(MPT,MHT),
     &IATYPNDX(MAC,MBT),IBRUKLC(MHC,MHT),IBRUKLW(MDW,MWS),IBRUKLF(MHT),
     &IBTYPNDX(MBT),IBTYP

C Strings
      COMMON/SBEM02/FUELNAME(MFT),SYSNAME(MPT),HSYSNAME(MHT),BLDSS(MSS),
     &DHWGEN(MDW),BTYPNAME(MBT),SBTYP(MSBT),BLDREG(MREG),BLDSTG(MSTG),
     &ATYPNAME(MAC),TLIGHT(MILS)

C Real numbers
      COMMON/SBEM03/FUELCO2(MFT),SYSEFFC(MCS,MHT),DHWEFF(MDW,2),
     &SYSEFF(MPT),BLDIF(MSS),BLDLZC(MSS),PDRL(MILS,2),VERSBEM,SFPDEF

C NCM common blocks for read/write of *.ncm project specific file and
C for BRUKL input file (*.inp)
C Integers. 
      integer theactivityindex   ! the long list (up to 525) activity index for each zone.
      COMMON/SBEM04/IBRUKH(MNS),IBRUKC(MNS),IBRUKW(MNS),IBRUKF(MNS),
     &IHGEF(MNS),ICGEF(MNS),IFTYP(MNS),IHLZ(MNS),IDHWS(MNS),IDHFL(MNS),
     &INCMSYS(MNS),IDHWLZ(MNS),IACTYTYP(MNS),ILITYP(MNS),
     &ILIGHTUSER(MNS),ISBEM,IBUSERTYP,IBSS,IRGG,ISTG,ISBT,NCMSYS,INOTI,
     &NDHWSYS,theactivityindex(MNS)

C Strings
      COMMON/SBEM05/DHWNAME(MNS),HVACNAME(MNS),LASBEM,SBREF,APCHK,
     &PJNAME,BADDRESS,BCITY,OWNERNAME,OWNERTEL,BUILDINGPC,OWNERADDR,
     &OWNERCITY,OWNERPC,CERTIFNAME,CERTTEL,CERTADDRESS,CERTCITY,CERTPC

C Real numbers
      COMMON/SBEM06/ROOFFR,WALLFR,BINF50,ROOFFRFR,WALLFRFR,HGEF(MNS),
     &CGEF(MNS),SFPHS(MNS),HWEF(MNS),ACH(MNS),LIGHTWATTAGE(MNS)

      COMMON/SBEM07/BEREMSN,AEREMSN,BERHEAT(MNS),BERCOOL(MNS),
     &BERLIGHT(MNS),BERDHW(MNS),BERAUX(MNS),BERMACH(MNS),AERHEAT(MNS),
     &AERCOOL(MNS),AERLIGHT(MNS),AERDHW(MNS),AERAUX(MNS),AERMACH(MNS)

C Activities global list
      INTEGER bld_order_index ! building order indices from the *bldg_type_start section of the file
      REAL occupant_dens  ! occupant density people/m2
      INTEGER metabolic_rate ! metabolic rate W/person/m2
      REAL fresh_air      ! outside air per person litres/sec/person
      INTEGER lighting_lux   ! lighting lux
      REAL equip_gain     ! equipment W/m2 
      REAL dhw_litres     ! domestic hot water litres/day/m2
      INTEGER latent_ocup_percent ! occupant latent percentage (of the whole gain)
      INTEGER latent_equip_percent ! equipment latent percentage (of the whole gain)
      REAL display_lighting ! lighting display W/m2
      REAL Hmainsetpoint,Cmainsetpoint  
      common/actglob/bld_order_index(MACL),occupant_dens(MACL),
     &  metabolic_rate(MACL),fresh_air(MACL),lighting_lux(MACL),
     &  equip_gain(MACL),dhw_litres(MACL),latent_ocup_percent(MACL),
     &  latent_equip_percent(MACL),display_lighting(MACL),
     &  Hmainsetpoint(MACL),Cmainsetpoint(MACL)

      character*72 roomactname ! the quoted string naming the activity
      character*102 roomactdoc  ! a quote description of activity
      common/actglobtext/roomactname(MACL),roomactdoc(MACL)

C Project specific *.ncm file variables
      INTEGER ISBEM     ! a value of 1 signals that isbem data exists
      INTEGER IBUSERTYP ! this matches the list of buildings in isbem.
      INTEGER IBSS      ! building service strategy
      INTEGER IRGG      ! building regulations to follow
      INTEGER ISTG      ! building design stage
      INTEGER ISBT      ! Scottish building types
      INTEGER INCMSYS   ! Index of HVAC system type (read from SBEM db)
      INTEGER NCMSYS    ! total number of HVAC system types in model
      INTEGER NDHWSYS   ! total number of DHW generators in model
      INTEGER IHGEF     ! index of heat generator (read from SBEM db)
      INTEGER ICGEF     ! index of cool generator (read from SBEM db)
      INTEGER IFTYP     ! index number of fuel type for heat generator
      INTEGER IHLZ      ! zone to HVAC system coupling index
      INTEGER IDHWS     ! index of DHW generators in the model
      INTEGER IDHFL     ! index of fuel type for DHW generator
      INTEGER IDHWLZ    ! zone to DHW coupling index, 0 if no DHW in that zone
                        ! +N if coupled with DHW system number N
                        ! -N if coupled with HVAC system number N
      INTEGER IACTYTYP  ! activity type index for each zone
      INTEGER ILITYP    ! lighting type index for each zone
      INTEGER ILIGHTUSER ! flag for user defined lighting wattage
      INTEGER INOTI     ! if set to 1 this is a notional model 
                          
      CHARACTER*72 LASBEM   ! SBEM project specific file name (*.ncm)
      CHARACTER*3  SBREF    ! Scottish Accredited Construction Details followed (yes/no)
      CHARACTER*3  APCHK    ! Compliance of air permeabilty to be checked (yes/no)
      CHARACTER*30 DHWNAME  ! Names of DHW generators in the model
      CHARACTER*30 HVACNAME ! Names of HVAC systems in the model
      CHARACTER*64 PJNAME       ! Project name
      CHARACTER*64 BADDRESS     ! Building address
      CHARACTER*64 BCITY        ! Building in which city?
      CHARACTER*64 OWNERNAME    ! Owner name
      CHARACTER*64 OWNERTEL     ! Owner telephone number
      CHARACTER*64 BUILDINGPC   ! Building post code
      CHARACTER*64 OWNERADDR    ! Owner address
      CHARACTER*64 OWNERCITY    ! Owner in which city?
      CHARACTER*64 OWNERPC      ! Owner post code
      CHARACTER*64 CERTIFNAME   ! Certifier name
      CHARACTER*64 CERTTEL      ! Certifier telephone number
      CHARACTER*64 CERTADDRESS  ! Certifier address
      CHARACTER*64 CERTCITY     ! Certifier in which city?
      CHARACTER*64 CERTPC       ! Certifier post code

      REAL ROOFFR   ! roof glazing fraction required
      REAL WALLFR   ! wall glazing fraction required
      REAL ROOFFRFR ! roof window frame fraction required
      REAL WALLFRFR ! wall window frame fraction required
      REAL BINF50   ! building's permeability at 50Pa in m3/m2.hour
      REAL HGEF     ! heat generator efficiency/COP
      REAL CGEF     ! cool generatr efficiency/COP
      REAL SFPHS    ! Specific fan power SFP for ventilation W/l/s
      REAL HWEF     ! DHW generator efficiency
      REAL ACH      ! Air changes per zone
      REAL LIGHTWATTAGE ! Heat gains (Watts) per zone in the cases of user specifies them
      REAL BEREMSN  ! total CO2 emission of real building
      REAL AEREMSN  ! total CO2 emission of notional building
      REAL BERHEAT  ! heating CO2 emission of real building
      REAL BERCOOL  ! cooling CO2 emission of real building
      REAL BERLIGHT ! lighting CO2 emission of real building 
      REAL BERDHW   ! DHW CO2 emission of real building
      REAL BERAUX   ! auxiliary CO2 emission of real building
      REAL BERMACH  ! equipment CO2 emission of real building
      REAL AERHEAT  ! heating CO2 emission of notional building
      REAL AERCOOL  ! cooling CO2 emission of notional building
      REAL AERLIGHT ! lighting CO2 emission of notional building 
      REAL AERDHW   ! DHW CO2 emission of notional building
      REAL AERAUX   ! auxiliary CO2 emission of notional building
      REAL AERMACH  ! equipment CO2 emission of notional building

c      COMMON/SBEM07/BER,AER,BERHEAT(MNS),BERCOOL(MNS),BERLIGHT(MNS),
c     &BERDHW(MNS),BERAUX(MNS),BERMACH(MNS),AERHEAT(MNS),AERCOOL(MNS),
c     &AERLIGHT(MNS),AERDHW(MNS),AERAUX(MNS),AERMACH(MNS)
C SBEM database variables
      INTEGER IBTYP     ! this matches the index value in NCM guide table 4
      INTEGER ISYSAPP   ! hash table with elements equal to 1 if that heating or
                        ! cooling generator is applicable to that HVAC system
      INTEGER IBRUKLH   ! BRUKL HVAC-HEAT index number
      INTEGER IBRUKLC   ! BRUKL HVAC-COOL index number
      INTEGER IBRUKLW   ! BRUKL HVAC-DHW index number (domestic hot water)
      INTEGER IBRUKLF   ! BRUKL HVAC-SFP index number (specific fan power)
      INTEGER IBTYPNDX  ! Building type index number
      INTEGER IATYPNDX  ! Activity type index number hash table

      CHARACTER*42 FUELNAME ! Name of fuel used
      CHARACTER*60 SYSNAME  ! heating or cooling system name
      CHARACTER*70 HSYSNAME ! HVAC system name
      CHARACTER*25 DHWGEN   ! DHW generator name
      CHARACTER*40 BLDSS    ! building services strategy
      CHARACTER*50 BTYPNAME ! Building type name
      CHARACTER*40 SBTYP    ! Scottish building type
      CHARACTER*50 BLDREG   ! Building regulations to follow
      CHARACTER*12 BLDSTG   ! Building design stage (as built or as designed)
      CHARACTER*40 ATYPNAME ! Activity type name
      CHARACTER*70 TLIGHT   ! Lighting type name

      REAL VERSBEM  ! SBEM db version no.
      REAL SFPDEF   ! default specific fan power (units?)
      REAL FUELCO2  ! CO2 emission rating for fuel
      REAL SYSEFFC  ! cooling mechanism COP for cooling system
      REAL SYSEFF   ! heating or cooling system seasonal efficiency or
                    ! coefficient of performance
      REAL DHWEFF   ! DHW generator efficiency before 1998 array index 1
                    ! and after 1998 (array index 2)
      REAL BLDIF    ! building improvement factor
      REAL BLDLZC   ! building low and zero carbon benchmark
      REAL PDRL     ! power density (W/m2)/100lux for lighting types PDRL(?,1)
                    ! for commercial & PDRL(?,2) for industrial use