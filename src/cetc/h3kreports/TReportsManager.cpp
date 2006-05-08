
#ifdef _WIN32
#pragma warning (disable: 4786) //to disable annoying "identifier was truncated to '255' characters in the debug information" in VC++
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>

#include "sys/stat.h"

#include "TReportsManager.h"
#include "TXMLAdapter.h"


#define DEBUG 0

#define SUMMARY 0
#define LOG     1
#define STEP    2


using namespace std;

std::string gString;


//Yee old trim function..
std::string trim(std::string const& source, char const* delims = " \t\r\n") {
  //cout << "trim_: " << std::endl;
  std::string result(source);
  std::string::size_type index = result.find_last_not_of(delims);
  if(index != std::string::npos)
    result.erase(++index);

  index = result.find_first_not_of(delims);
  if(index != std::string::npos)
    result.erase(0, index);
  else
    result.erase();

  return result;

}

extern "C"
{
  //These are the calls advertised to Fortran. Note that they are all
  //lowercase and end in an underscore. Calls from Fortran must omit
  //the trailing underscore

  /**
   *  Perform binning operations and move to next timestep
   */
   
  void rep_update__(long* step, float* hour, long* day, bool* bStartup)
  {
    if(DEBUG) cout << "rep_update_: " << *step << "\t"
                   << *hour << "\t" << *day << endl;
    TReportsManager::Instance()->Update( *step, *hour, *day, *bStartup );
  }

  /**
   * Save data passed from bps
   *
   * note: sPassedValue and sPassedName are switched from the C++ call because
   * Fortran strings need to be at the end of a CALL - go figure
   *
   */
  void rep_report__(float *value, char *name, int sPassedName_length)
  {
    std::string passedName = std::string(name, sPassedName_length);

    if(DEBUG) cout << "rep_report_: " << sPassedName_length
                   << " \"" << *value << "\"\t\"" << passedName
                   << "\"" << endl;
    TReportsManager::Instance()->Report(passedName, *value);
  }


  /**
   * Text summary (obsolete)
   */
  void rep_summary__(  )
  {
    TReportsManager::Instance()->OutputSummary();
  }


  /**
   *  Output results in xml/csv formats
   */

  void rep_xml_summary__( )
  {
    // produce a dictionary of results
    TReportsManager::Instance()->OutputDictionary();
    // produce out.xml
    TReportsManager::Instance()->OutputXMLSummary();
    // produce out.csv
    TReportsManager::Instance()->OutputCSVData();
    // produce out.summary
    TReportsManager::Instance()->OutputTXTsummary();

  }

  /**
   *  attach metadata to a variable
   */

  void rep_set_meta__(char *sVarName, char *sMetaName, char *sMetaValue,
                      int sVarNameLength, int sMetaNameLength, int sMetaValueLength)
  {
   std::string varName =std::string(sVarName, sVarNameLength);
   std::string metaName =std::string(sMetaName, sMetaNameLength);
   std::string metaValue =std::string(sMetaValue, sMetaValueLength);

    //if(DEBUG) cout << "rep_meta_: " << varName << "\t" << metaName
    //             << "\t" << metaValue << endl;
    TReportsManager::Instance()->SetMeta(varName, metaName, metaValue);
  }

  /** 
   *  Set configuration file
   */
  void rep_set_config_file__(char *sFilePath, int sPathLength)
  {
   std::string str =std::string(sFilePath, sPathLength);

    TReportsManager::Instance()->ParseConfigFile(str);

  }

  /**
   *  Set a parameter
   */
   void rep_set_parameter__(char *sParamName,
                           char *sParamValue,
                           int sNameLength,
                           int sValueLength)
  {
    std::string paramName =std::string(sParamName, sNameLength);
    std::string paramValue =std::string(sParamValue, sValueLength);
    
    TReportsManager::Instance()->SetReportParameter(paramName, paramValue);
  
  }
  
  /**
   *  Return boolian indicating if output is enabled.
   */
  bool bh3k_rep_enabled__(){
    return TReportsManager::Instance()->ReportsEnabled();
  }

  /**
   *   Enable / disable output 
   */
  void h3k_enable_reports__( bool& bNewStatus ){
    TReportsManager::Instance()->EnableReports(bNewStatus);
    return;
  }
  
  /** 
   * return a boolian indicating H3kreports is active
   */
  bool rep_xmlstatus__(){
    return true;
  }

  /**
   * Return the current value of a configuration parameter
   *
   */
   void rep_report_config__(char* sValue, int iValLength, char *sParam, int iNameLength){


      short iPos;
      std::string paramValue;
      std::string paramName; 

      paramName = std::string(sParam, iNameLength);
      
      
      TReportsManager::Instance()->ReportConfig(paramName,paramValue);
   
      strcpy (sValue, paramValue.c_str());

      for (iPos = paramValue.size(); iPos<=iValLength; iPos++){
         sValue[iPos]=' ';
      }
   
      return;
   }
  /** 
   * Return the current status of an item in a vector 
   *
   */
   bool rep_report_list__(char *sType, char *sSheet,
                          int iTypeLength, int iSheetLength){

      std::string paramValue;
      std::string paramType;


      paramType  = std::string(sType, iTypeLength);
      paramValue = std::string(sSheet, iSheetLength);

      
      return TReportsManager::Instance()->ReportList(paramType,paramValue);
   
   }
   
   /**
    * Update the value of a configuration parameter
    *
    */
    void rep_update_config__(char* sParam, char* sValue, int iNameLength, int iValLength){

      std::string paramName = std::string(sParam, iNameLength);
      std::string paramValue = std::string(sValue, iValLength);

      TReportsManager::Instance()->UpdateConfig(paramName, paramValue);
      
    }
   /**
    * Toggle the status of an on-off configuration parameter
    */
    bool rep_toggle_config__(char* sParam, int iNameLength){

      std::string paramName = std::string(sParam, iNameLength);

      return TReportsManager::Instance()->ToggleConfig(paramName);

    }


    /**
     *   Write out configuration file with new options
     */
     
    void rep_update_config_file__(){

      TReportsManager::Instance()->UpdateConfigFile();
    
    }
    
  
/**
* This is a faster function to add a report for a value in ESP-r.
  There is no need  for LNBLNK to be used on any of the character arguments
  on the fortran side. This replaces rep_set_meta and rep_report..This will
  reduce simtulation times. Note there still has to be some major overhaul on
  the data structure itself (Like each zone, system and potentially surface
  entity has to be unique . This is problematic when trying to write genteric
  xslt transforms..So new functionality will be add as time goes on..So proper
  versioning of the xml file is required.

* @param value The float value to be stored and reported on.
* @param sVarName The Unique xml path location to where the data is to be stored.
* @param sMetaName The metatag to be added. (usually 'units' is used)
* @param sMetaValue The value for the above metatag (Usually the unit type, like
                    (W) for watts)
* @param sDescription A detailed description of the variable being reported on.
                    Will be outputted with the dictionary.
* @param *Length The length of the corrosponding char arrays.

**/

void add_to_xml_reporting__(float* value,
                        char* sVarName,
                        char* sMetaName,
                        char* sMetaValue,
                        char* sDescription,
                        int sVarNameLength,
                        int sMetaNameLength,
                        int sMetaValueLength,
                        int sDescriptionLength)
{
   std::string varName =std::string(sVarName, sVarNameLength);
   std::string metaName =std::string(sMetaName, sMetaNameLength);
   std::string metaValue =std::string(sMetaValue, sMetaValueLength);

   /* Temporary code to permit comparison of new xml output with old
      output */

   /* if ( trim(metaValue) == "(W)" ) {
      TReportsManager::Instance()->SetMeta (varName, "WattsToGJ", "");
   } */

   // Note: SetMeta does not need to be called with every report. Improvements
   // possible!
   
   TReportsManager::Instance()->SetMeta(varName, metaName, metaValue);
   TReportsManager::Instance()->Report(varName, *value);



  }

}

// Prototypes

bool testForMatch( const std::vector<std::string>& txtlist, const std::string& search_text);

bool SearchVars( const std::vector<std::string>& txtlist,
                 const std::string& search_text,
                 TVariableData& Variable,
                 int mode  );

bool SearchAllVars( const std::vector<std::string>& txtlist1,
                    const std::vector<std::string>& txtlist2,
                    const std::vector<std::string>& txtlist3,
                    const std::string& search_text,
                    TVariableData& Variable);

TReportsManager* TReportsManager::ptr_Instance = NULL;

bool TimeDataInRange(const TTimeData& data, const TTimeDataRange& range)
{
  if((range.begin.month < data.month) || (range.end.month > data.month))
    return false;

  if(range.begin.month == data.month)
    {
      if(range.begin.day > data.day) return false;
      else if(range.begin.day == data.day)
        {
          if(range.begin.hour > data.hour) return false;
          else if(range.begin.hour == data.hour)
            {
              if(range.begin.step > data.step) return false;
            }
        }
    }

  if(range.end.month == data.month)
    {
      if(range.end.day < data.day) return false;
      else if(range.end.day == data.day)
        {
          if(range.end.hour < data.hour) return false;
          else if(range.end.hour == data.hour)
            {
              if(range.end.step < data.step) return false;
            }
        }
    }

  return true;
}

TReportsManager* TReportsManager::Instance(  )
{
  if(ptr_Instance == NULL) //create a new instance
    {
      ptr_Instance = new TReportsManager();
    }
  return ptr_Instance;
}

// Constructor
TReportsManager::TReportsManager(  )
{
  m_currentMonth = 0;
  m_step_count = 0;
  m_currentTime.step = 0;
  m_currentTime.hour = 0;
  m_currentTime.day = 0;
  m_currentTime.month = 0;
  
  ParseConfigFile("input.xml"); //default input file
  SetFlags();                   //set config flags & defaults
}


/**
 * Update all data at the end of a timestep, and perform avg/bin operations
   operations as necessary
 */
 
bool TReportsManager::Update(long step, float hour, long day, bool bStartup)
{

  if ( ! bReports_Enabled ) return false;

  static bool firstCallToUpdate = true;
  bool yearRollOver = false;
  VariableDataMap::iterator pos;
  // if simulation is still in start-up, and output of
  // start-up data not requested, return without doing anything.
  if ( bStartup && ! bReportStartup){return true;}

  if(DEBUG) cout << "Update step: " << step << "  hour: " << hour << "  day: "
                 << day << endl;

  if(firstCallToUpdate) //init on first call
    {
      m_step_count = 0;
      if(DEBUG) cout << "First call" << endl;
      
      m_currentTime.step = step - 1; // -1 to force an update this call
      
      m_currentTime.hour = (int)hour;
      m_currentTime.day = day;

      //set month
      m_currentTime.month = 0;
      while(m_currentTime.day > kMonthlyTimesteps[m_currentTime.month])
        ++m_currentTime.month;

      firstCallToUpdate = false;
    }

  if(day - m_currentTime.day < 0) yearRollOver = true;



  //add new hour
  if((int)hour != m_currentTime.hour)
    {
      for(pos  = m_variableDataList.begin();
          pos != m_variableDataList.end(); ++pos)
        if(SearchAllVars(m_nodes,
                         m_step_nodes,
                         m_summary_nodes,
                         pos->first,
                         pos->second))
          pos->second.UpdateHourly();

      m_currentTime.hour = (int)hour;

      if(DEBUG) cout << "Rollover hour: " << m_currentTime.hour << endl;
    }

  //add a new day
  if(day != m_currentTime.day)
    {
      for(pos  = m_variableDataList.begin();
          pos != m_variableDataList.end(); ++pos)
        if(SearchAllVars(m_nodes,
                        m_step_nodes,
                        m_summary_nodes,
                        pos->first,
                        pos->second))
          pos->second.UpdateDaily();

      m_currentTime.day = day;
      if(DEBUG) cout << "Rollover day: " << m_currentTime.day << endl;
    }

  //add a new month
  if((m_currentTime.day > kMonthlyTimesteps[m_currentTime.month]) || yearRollOver)
    {
      for(pos = m_variableDataList.begin();
          pos != m_variableDataList.end(); ++pos)
        if( SearchAllVars(m_nodes,
                         m_step_nodes,
                         m_summary_nodes,
                         pos->first,
                         pos->second))
          pos->second.UpdateMonthly(); 

      m_currentTime.month = (m_currentTime.month + 1)%12;

      if(DEBUG) cout << "Rollover month: " << m_currentTime.month << endl;
    }

  //add a new year - not used right now
  /*    if(m_currentTime.month == m_rolloverTime.month)
        {
        for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos)
        pos->second.UpdateAnnual();

        //m_currentTime.month = 0;
        }*/

  //check user-defined spans
  for(unsigned int i = 0; i < m_userDefinedTime.size(); i++)
    {
      if(TimeDataInRange(m_currentTime, m_userDefinedTime[i]))
        {

          for(pos  = m_variableDataList.begin();
              pos != m_variableDataList.end(); ++pos)
            if((m_nodes.size() == 0) || SearchAllVars(m_nodes,
                                                      m_step_nodes,
                                                      m_summary_nodes,
                                                      pos->first,
                                                      pos->second))
              pos->second.UpdateUserDefined();
        }
    }

  /* check to see if the time step has changed, and if so,
     update data list. Note: these calls must come after
     the above roll-over code. Otherwise, the data will be
     assigned to the hour/day/month/year current bin, before
     rollover functions have determined of a new bin is required.
  */
  if(step != m_currentTime.step)
    {
      for(pos  = m_variableDataList.begin();
          pos != m_variableDataList.end(); ++pos)
        if((m_nodes.size() == 0) || SearchAllVars(m_nodes,
                                                  m_step_nodes,
                                                  m_summary_nodes,
                                                  pos->first,
                                                  pos->second))
          pos->second.Update();
      m_currentTime.step = step;
    }

  m_step_count++;

  return true;
}

void TReportsManager::AddUserDefinedTimeRange(TTimeDataRange range)
{
  m_userDefinedTime.push_back(range);
}

/**
 * Take data passed from bps and stuff it into the approprate vector
 */
void TReportsManager::Report( const std::string& sPassedName, const double& sPassedValue )
{
     if ( ! bReports_Enabled ) return;
        
     m_variableDataList[sPassedName].Set(sPassedValue,bTS_averaging, m_step_count);


}

/**
 * Set meta parameters, such as the units associated with a variable
 */
 
void TReportsManager::SetMeta(const std::string& sName, const std::string& sMetaName, const std::string& sMetaValue)
{

  if ( ! bReports_Enabled ) return;
  
  m_variableDataList[sName].SetMeta(sMetaName, sMetaValue);
  

}

/**
 * Function returing the current status of reporting output
 */
bool TReportsManager::ReportsEnabled(){
   return bReports_Enabled;
}

/**
 * Function setting the status of reporting output
 */
void TReportsManager::EnableReports( bool& ReportsStatus ){
   bReports_Enabled = ReportsStatus;
   return;
}


bool TReportsManager::OutputSummary()
{
  return OutputSummary("out.txt");
}

bool TReportsManager::OutputSummary( const std::string& outFilePath )
{

  if ( ! bReports_Enabled ) return false;

  if(m_inputFilePath == "") //by default, we dump evverything
    {
      Log();
    }

  return true;
}





void TReportsManager::Log(  )
{

  if ( ! bReports_Enabled ) return;

  VariableDataMap::iterator pos;
  for( pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos)
    {
      cout << "Outputing data for: " << trim(pos->first) << endl;
      pos->second.Log();
    }
}

void TReportsManager::OutputXMLSummary()
{

  if ( ! bReports_Enabled ) return;

  // Write out out.xml
  OutputXMLSummary("out.xml");
  std::string Target;

  // Check to see that i) a valid style sheet was provided and
  // ii) transforms were requested.
  if (  bStyleSheetGood &&  bTransformXMLRequested ) {

    // check if target has been provided.
    if ( m_params["transform_destination_file"].empty() ){
      Target = "results.txt";
    }else{
      Target = m_params["transform_destination_file"];
    }

    TXMLAdapter XMLAdapter;

    // Synopsys :
    // WriteTransformedXML( source , target, stylesheet list )
    XMLAdapter.WriteTransformedXML( std::string("out.xml"), Target,  m_stylesheet_list );
  }

}

std::vector<std::string> PathComponents( std::string  path)
{
  vector<std::string> comp;
  size_t index, last_index;

  last_index = 0;
  if(path[path.size()-1] == '/') path.erase(path.size()-1); //remove trailing slash

  while((index = path.find("/", last_index + 1)) != std::string::npos)
    {
      if(path[last_index] == '/') ++last_index;
      comp.push_back(path.substr(last_index, index - last_index));
      last_index = index;
    }

  return comp;
}

///Streams report to an XML output file.

void TReportsManager::OutputXMLSummary(  const std::string& outFilePath )
{

  if ( ! bReports_Enabled ) return;
  
  //determine which variables to log
  bool useFlatHierarchy;
  // only dump xml if there's xml to dump!
  if(m_nodes.size() >> 0 || bDumpEverything ){

    if(m_params["hierarchy"].empty() || m_params["hierarchy"] == "tree")
      useFlatHierarchy = false;
    else
      useFlatHierarchy = true;

    TXMLAdapter doc;
    doc.AddNode(NULL, "system", "");

    // if style sheet provided and embedded link requested,
    // link style sheet.
    if(!m_stylesheet_list.empty() &&  bLinkStyleSheet )
    //set it to the first stylesheet in list only.
      doc.SetStylesheet(m_stylesheet_list[0]);

    doc.AddAttribute(doc.RootNode(), "version", "1.0");
    VariableDataMap::iterator pos;

    std::string currentPath = "";
    for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos)
      {
        if(bDumpEverything ||  SearchVars(m_nodes,
                                          pos->first,
                                          pos->second,
                                          LOG))
          {
            TXMLNode currentNode = doc.RootNode();

            if(useFlatHierarchy)
              {
                currentNode = doc.AddNode(currentNode, "parameter", "");
                doc.AddNode(currentNode, "name", trim(pos->first));
              }
            else
              {
                vector<std::string> comp = PathComponents(trim(pos->first));
                for(unsigned int i = 0; i < comp.size(); i++)
                  {
                    vector<TXMLNode> nextNodeVector = doc.GetChildren(currentNode, comp[i]);
                    if(nextNodeVector.size() == 0)
                      currentNode = doc.AddNode(currentNode, comp[i], "");
                    else
                      currentNode = nextNodeVector[0];
                  }

                currentNode = doc.AddNode(currentNode, trim(pos->first).substr(trim(pos->first).find_last_of("/") + 1), "");
                doc.AddNode(currentNode, "name", trim(pos->first).substr(trim(pos->first).find_last_of("/") + 1));
              }
            //tell it to output step data if required
            //if( testForMatch(m_step_nodes, pos->first) )
            //  SetMeta(pos->first, "StepData", "");
            //find(m_step_nodes.begin(), m_step_nodes.end(), pos->first) != m_step_nodes.end())


            pos->second.OutputXML(&doc, currentNode, m_params);
          }
      }

    doc.WriteToFile(outFilePath);
  }
}

/*
 * Ouptput Dictionary simply dumps a listing of all valid tags
 * encountered during a simulation run.
 *
 */
bool TReportsManager::OutputDictionary()
{
  return OutputDictionary("out.dictionary");
}
bool TReportsManager::OutputDictionary( const std::string& outFilePath )
{

  if ( ! bReports_Enabled ) return false;

  VariableDataMap::iterator pos;

  ofstream dictionaryFile;
  if ( m_variableDataList.size() >> 0 && bDumpDictionary ){

    dictionaryFile.open(outFilePath.c_str());

    for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos) {
      dictionaryFile << trim(pos->first) << "\n";
    }
    dictionaryFile.close();
  }
  return true;
}


/**
 * OUTPUT summary is an alternative method for producing
   a token-value list of data without performing a transform
   on the xml output.
 *
 */
bool TReportsManager::OutputTXTsummary()
{
  return OutputTXTsummary("out.summary");
}

bool TReportsManager::OutputTXTsummary( const std::string& outFilePath )
{

  if ( ! bReports_Enabled ) return false;

  VariableDataMap::iterator pos;
  ofstream summaryFile;
  summaryFile.setf(ios::showpoint);
  summaryFile.precision(8);


  if (m_summary_nodes.size() >> 0){

    // open summary file
    summaryFile.open(outFilePath.c_str());

    // loop through all nodes, and check if summary data has been requested.
    for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos){
      if ( SearchVars(m_summary_nodes,
                      pos->first,
                      pos->second,
                      SUMMARY) ){
        // summary output has been requested. Open file, if necessary
        summaryFile <<  pos->second.OutputTXT( trim(pos->first) ,m_params );
      }
    }

    // close file
    summaryFile.close();
  }
  return true;
}
/**
 * OUTPUT CSV data is an alternative method for producing CSV
 * formatted output without transforming XML. Permits time-step
 * data to be produced from h3kreports on plantforms without
 * XSLT2.0 support.
 *
 */

bool TReportsManager::OutputCSVData()
{
  return OutputCSVData("out.csv");
}

bool TReportsManager::OutputCSVData( const std::string& outFilePath )
{

  if ( ! bReports_Enabled ) return false ;
  
  VariableDataMap::iterator pos;
  unsigned int curr_step,step_start;
  bool fileopen;

  std::string temp_text;

  ofstream csvFile;
  csvFile.setf(ios::showpoint);
  csvFile.precision(8);

  /* Check if time-step averaging has been set. If so, skip first (unaveraged)
   * step.
   *
   */

  if ( bTS_averaging ) {
    //skip first step
   step_start = 1;
  }else{
    step_start = 0;
  }

  fileopen = 0;

  // write out header row (names)
  for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos) {

    if( SearchVars(m_step_nodes,
                   pos->first,
                   pos->second,
                   STEP) ){
      // open file if necessary

      if (!fileopen) {
        csvFile.open(outFilePath.c_str());
        fileopen = 1;
      }


      // Add spaces to make header more "speadsheet friendly"
      temp_text = trim(pos->first);
      while ( temp_text.find("/") != string::npos ){
         temp_text.replace( temp_text.find("/"), 1, " : ");
      }
      while ( temp_text.find("_") != string::npos ){
         temp_text.replace( temp_text.find("_"), 1, " ");
      }


      csvFile << temp_text;  //pos->first;
      csvFile << ", ";
    }
  }
  csvFile << "\n";

  // write out units
  for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos) {
    if( SearchVars(m_step_nodes,
                   pos->first,
                   pos->second,
                   STEP) ){
      csvFile << pos->second.RetrieveMeta("units");
      csvFile << ", ";
    }
  }
  csvFile << "\n";

  // loop through all timesteps
  for (curr_step = step_start; curr_step <  m_step_count ; curr_step++){
    // loop through all variables
    for(pos = m_variableDataList.begin(); pos != m_variableDataList.end(); ++pos) {
      // check if time-step data has been requested for variable
      if( SearchVars(m_step_nodes,
                     pos->first,
                     pos->second,
                     STEP) ){

        // time-step data has been requested. Stream out.

        csvFile << pos->second.RetrieveValue(curr_step);
        csvFile << ", ";

      }
    }
    // End of variable list. New line
    csvFile << "\n";
  }

  csvFile.close();

  return true;
}


/**
 * SetReportParameter: collect & store configuration data passed from bps
 *
 *
 */

void TReportsManager::SetReportParameter(const std::string& param, const std::string& value)
{

  if ( ! bReports_Enabled ) return;

  m_params[param] = value;
  // Is parameter esp-r's default timestep averaging setting?
  if ( param == "default_time_step_averaging" ) {
    if ( m_params["time_step_averaging"] == "true" ||
        m_params["time_step_averaging"] == "false"     ){
       // TS averaging has been set in H3K reports configuration. Do nothing.
    }else if ( strtol(m_params["default_time_step_averaging"].c_str(), NULL, 10) == 1 ){
      // TS averaging has not been set in H3KReports configuration. Use
      // ESP-r setting
      bTS_averaging = true;
    }else {
      // TS averaging has not been set in H3KReports configuration. Use
      // ESP-r default.
      bTS_averaging = false;
    }
  }
}


/**
 * Write out a configuration file using whatever options
 * that are in memory.
 */
void TReportsManager::UpdateConfigFile(){

   std::vector<std::string>::iterator sheet;
   std::vector<std::string>::iterator var;
   std::map<std::string, std::string>::iterator param;
   std::string sTemp;
   
   TXMLAdapter inputXML ;

   // Set defaults if required.
   SetFlags();

   inputXML.AddNode(NULL, "configuration", "");
   TXMLNode currentNode = inputXML.RootNode();

   // Loop through all defined parameters, add to xml document 
   for( param = m_params.begin(); param != m_params.end(); param++)
   {
      // Don't write out unset parameters
      if ( param->second != ""){
         inputXML.AddNode(currentNode,param->first,param->second);
      }
   }
   
   // Loop through stylesheets, and add to xml document 
   for (sheet  = m_stylesheet_list.begin();
        sheet != m_stylesheet_list.end();
        sheet++)
   {
    sTemp = *(sheet);
    inputXML.AddNode(currentNode,"style_sheet",sTemp.c_str());
  
   }

   // Loop through output variables and add to document
   for ( var = m_nodes.begin(); var != m_nodes.end(); var++ ){
      sTemp = *(var);
      inputXML.AddNode(currentNode,"log_variable",sTemp.c_str());
   }
   
   // Loop through timestep output variables and add to document
   for ( var = m_step_nodes.begin(); var != m_step_nodes.end(); var++ ){
      sTemp = *(var);
      inputXML.AddNode(currentNode,"step_variable",sTemp.c_str());
   }

   // Loop through summary output variables and add to document
   for ( var = m_summary_nodes.begin(); var != m_summary_nodes.end(); var++ ){
      sTemp = *(var);
      inputXML.AddNode(currentNode,"summary_variable",sTemp.c_str());
   }

   // Write XML document ot disk
   inputXML.WriteToFile(m_inputFilePath);   
}


/**
 * Parse the configuration file (input.xml)
 *
 */

void TReportsManager::ParseConfigFile( const std::string& filePath  )
{
  TXMLAdapter inputXML(filePath);
  
  m_inputFilePath = filePath;

  
  if(inputXML.RootNode() == NULL)
  {
      bReports_Enabled = false;
      return;
  
  }else{
      
      bReports_Enabled = true;

  }

  // Nodes to appear in out.xml
  m_nodes = inputXML.GetNodeValues("log_variable", inputXML.RootNode());

  // Nodes to appear in out.csv
  m_step_nodes = inputXML.GetNodeValues("step_variable", inputXML.RootNode());

  // Nodes to appear in out.summary
  m_summary_nodes = inputXML.GetNodeValues("summary_variable", inputXML.RootNode());

    // Sytlesheet list for multiple transforms.
  m_stylesheet_list = inputXML.GetNodeValues("style_sheet", inputXML.RootNode());

  // Should style sheet be linked?
  m_params["link_style_sheet"] = inputXML.GetFirstNodeValue("link_style_sheet", inputXML.RootNode() );

  // XSLT transform requested
  m_params["apply_style_sheet"] = inputXML.GetFirstNodeValue("apply_style_sheet", inputXML.RootNode() );

  //Target for xslt transform
  m_params["transform_destination_file"] = inputXML.GetFirstNodeValue("transform_destination_file", inputXML.RootNode());

  //hierarchy type
  m_params["hierarchy"] = inputXML.GetFirstNodeValue("hierarchy", inputXML.RootNode());

  // minutes per timestep - This data isn't read out of input.xml.
  // bps sets "minutes_per_step" instead.
  // m_params["minutes_per_step"] = inputXML.GetFirstNodeValue("minutes_per_step", inputXML.RootNode());

  //overide for time-step averaging
  m_params["time_step_averaging"] = inputXML.GetFirstNodeValue("time_step_averaging", inputXML.RootNode());
  
  // concise reporting mode
  m_params["dump_all_data"] = inputXML.GetFirstNodeValue("dump_all_data", inputXML.RootNode());

  // Wildcards
  m_params["enable_xml_wildcards"] = inputXML.GetFirstNodeValue("enable_xml_wildcards", inputXML.RootNode());

  // Report start up period data
  m_params["report_startup_period_data"] = inputXML.GetFirstNodeValue("report_startup_period_data", inputXML.RootNode());

  // Dictionary output
  m_params["output_dictionary"] = inputXML.GetFirstNodeValue("output_dictionary", inputXML.RootNode());
  
  return;
}

/**
 * Review configuration parameters and set flags & defaults, as necessary
 *
 */

void TReportsManager::SetFlags(){

  // Test all stylesheets for readability
  // (We won't perform an xml transform with it if it's not there!)

  ifstream Style_Sheet_test;
  std::vector<std::string>::iterator sheet;

  // Loop through style sheets, and check if list is valid.  
  if ( ! m_stylesheet_list.empty() ){
    for( sheet = m_stylesheet_list.begin(); sheet < m_stylesheet_list.end(); sheet++)
    {

       std::string sTemp = *(sheet);
       
       Style_Sheet_test.open( sTemp.c_str(), ifstream::in );
       
       Style_Sheet_test.close();
       
       if ( Style_Sheet_test.fail() ) {
          
          Style_Sheet_test.clear(ios::failbit);
          
          m_stylesheet_list.erase(sheet);
       
       }else{

       }
    }
  }
  
  if ( ! m_stylesheet_list.empty() ) bStyleSheetGood = true;


  if ( m_params["hierarchy"].empty() ) {
    m_params["hierarchy"] = "flat";
  }

  // Check if stylesheet should be linked
  
  if ( m_params["link_style_sheet"]  == "true" ){
    bLinkStyleSheet = true;
  }else{
    m_params["link_style_sheet"] = "false";
    bLinkStyleSheet = false;
  }

  // Check if an XSLT transform is requested
  
  if ( m_params["apply_style_sheet"] == "true" ) {
    bTransformXMLRequested = true;
  }else{
    m_params["apply_style_sheet"] = "false";
    bTransformXMLRequested = false;
  }

  //Conscise reporting mode
 
  if ( m_params["dump_all_data"]  == "true" ){
    bDumpEverything = true;
  }else{
    m_params["dump_all_data"] = "false";
    bDumpEverything = false;
  }
  // wildcards
 
  if ( m_params["enable_xml_wildcards"]  == "true" ){
    bWildCardsEnabled = true ;
  }else{
    m_params["enable_xml_wildcards"] = "false";
    bWildCardsEnabled = false;
  }
  // Report results from start-up period?
  if ( m_params["report_startup_period_data"] == "true" ){
    bReportStartup = true;
  }else{
    m_params["report_startup_period_data"] = "false";
    bReportStartup = false;
  }
  
  // Dictionary output?
  if ( m_params["output_dictionary"] == "true" ){
    bDumpDictionary = true;
  }else{
    m_params["output_dictionary"] = "false";
    bDumpDictionary = false;
  }

  // Timestep averaging 
  if ( m_params["time_step_averaging"] == "false" ){
      bTS_averaging = false;
  }else if (m_params["time_step_averaging"] == "true" ){
      bTS_averaging = true;
  }
  
  return;
}

/**
 *    Function to determine if a parameter has been
 *    defined 
 *
 */

bool TReportsManager::ReportList( std::string cType,
                                  std::string cValue){
   bool bFound;

   // Search list of style sheets
   if ( cType == "style_sheet" ) {

      if (std::find(m_stylesheet_list.begin(),
                    m_stylesheet_list.end(),
                    cValue) != m_stylesheet_list.end()){
         bFound = true;

      }else{
         
         bFound = false;

      }

   // Other types of searches added here
   }else{         

   }

   return bFound;
}                                  
                                  


/**
 *   Return the value of a requested parameter
 *
 */
void TReportsManager::ReportConfig(std::string cParam,
                                   std::string &cValue){

   cValue = m_params[cParam];
   return;

}

/**
 *   Toggle value of requested parameter (true/false)
 *
 */
 

bool TReportsManager::ToggleConfig(std::string cParam){
   bool bSuccess;
   bSuccess = true;
   
   if ( m_params [ cParam ] == "true" ){
      m_params [ cParam ] = "false";
   }else{
      m_params [ cParam ] = "true";
   }
   
   return bSuccess; 
}

/**
 *   Update the value of a specified parameter
 *
 */
void TReportsManager::UpdateConfig(std::string cParam,
                                   std::string cValue ){
   
  
   // Add a style sheet to the stylesheet vector
   if ( cParam == "+style_sheet" &&
         std::find(m_stylesheet_list.begin(),m_stylesheet_list.end(), cValue)
         == m_stylesheet_list.end()){

        // empty the vector
        m_stylesheet_list.clear();
        // add new record
        m_stylesheet_list.push_back(cValue);

   // Drop a stylesheet from the vector
        
   }else if ( cParam == "-style_sheet" ){

         m_stylesheet_list.erase( std::find(m_stylesheet_list.begin(),
                                            m_stylesheet_list.end(),
                                            cValue) );

   // Otherwise, append value to m_params vector
   } else {
      m_params[cParam] = cValue;
   }

   // Reparse m_params and set flags used by TReportsManager
   SetFlags();
}

void TReportsManager::OutputTimestepData(  )
{

}


TTimeData TReportsManager::GetTimeInfo(  )
{
  return m_currentTime;
}

/**
 * Search through an array of strings, and check if any string matches search_text
 *
 */

bool TReportsManager::testForMatch(const std::vector<std::string>& txtlist,
                                   const std::string& search_text){

  Wildcard Cwildcard_engine;
  unsigned int txt;
  bool bResult = false;

  // Check if we've already flagged this data


  for (txt = 0; txt < txtlist.size() ; txt++ ) {

    // Wildcard searching
    if ( bWildCardsEnabled ){

      if ( Cwildcard_engine.wildcardfit(txtlist[txt].c_str(),search_text.c_str()) == 1) {
        bResult = true;
        return bResult;
      }

    } else if( txtlist[txt] == search_text ) {

      bResult = true;
      return bResult;

    }

  }

  return bResult;
}

/**
 * Search for a variable in an array of strings
 *
 */
bool TReportsManager::SearchVars( const std::vector<std::string>& txtlist,
                                  const std::string& search_text,
                                  TVariableData& Variable,
                                  int mode ){

  bool result;

  // If all data has been requested, return *match*
  if ( bDumpEverything ){

    return true;

  }else{

    // Check if search has been performed perviously
    if ( ! Variable.QuerySearchStatus(mode) ){

      // run search
      result = testForMatch( txtlist,  trim(search_text));

      // Save search result to ensure that we don't
      // have to run test-for-match again for this
      // variable!

      // update variable result
      Variable.UpdateSearchResult( mode, result);

      // Update search status
      Variable.UpdateSearchStatus( mode, true);

    }

    // return result
    return Variable.QuerySearchResult(mode);

  }

}

/**
 * Search for a variable in three arrays of strings
 *
 */

bool TReportsManager::SearchAllVars(const std::vector<std::string>& txtlist1,
                                    const std::vector<std::string>& txtlist2,
                                    const std::vector<std::string>& txtlist3,
                                    const std::string& search_text,
                                    TVariableData& Variable){

  if ( SearchVars(txtlist1, search_text, Variable, LOG )){
    return true;
  }
  if ( SearchVars(txtlist2, search_text, Variable, STEP  )){
    return true;
  }
  if ( SearchVars(txtlist3, search_text, Variable, SUMMARY )){
    return true;
  }
  return false;
}