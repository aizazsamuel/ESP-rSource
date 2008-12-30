
#include "TXMLAdapter.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <libxml/parserInternals.h>

#ifdef XSL
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

using namespace std;

TXMLAdapter::TXMLAdapter(string filePath)
{

        ifstream Input_file_test;

        // Test if input file exists.


        Input_file_test.open( filePath.c_str(), ifstream::in );
        Input_file_test.close();

        if (  Input_file_test.fail() ){

           m_document = NULL;

        }else{

            m_document = xmlParseFile(filePath.c_str());
            // Is file valid?
            if (m_document == NULL ){
               cerr << "\n H3KReports: configuration file ("<<filePath<<") does\n"
                    <<   " does not appear to be well-formed xml. H3K report output\n"
                    <<   " will be disabled. Check for errors in your input file, \n"
                    <<   " or perhaps build a new one using the menu option. \n\n";

            }else{

                if (xmlStrcmp(RootNode()->name, (const xmlChar *) "configuration")) {
                    fprintf(stderr,"input.xml document of the wrong type, root node != configuration");
                    xmlFreeDoc(m_document);
                    return;
                }
            }
       }
}

TXMLAdapter::TXMLAdapter()
{
        m_document = xmlNewDoc(BAD_CAST "1.0");
        
        if (m_document == NULL ) {
                cerr << "Document not created." << endl;
                return;
        }

}
void TXMLAdapter::SetStylesheet(std::string path)
{
        string content = "type=\"text/xsl\" href=\"" + path + "\"";
        TXMLNode pi = xmlNewPI(BAD_CAST "xml-stylesheet", BAD_CAST content.c_str());
        xmlAddPrevSibling(xmlDocGetRootElement(m_document),  pi);
}

TXMLNode TXMLAdapter::AddNode(TXMLNode parent, std::string name, std::string value)
{
        TXMLNode newNode;
        if(parent == NULL) //new root node
        {
                newNode = xmlNewNode(NULL, BAD_CAST name.c_str());
                xmlDocSetRootElement(m_document, newNode);
        }
        else
        {
                /*
                * xmlNewChild() creates a new node, which is "attached" as child node
                * of root_node node.
                */

	  //If this node has no content of its own..ensure a NULL is passed. 
	  if (value =="")
	    {
	      newNode = xmlNewChild(parent, NULL, BAD_CAST name.c_str(),
				    NULL);
	    }
	  else
	    {
	      newNode = xmlNewChild(parent, NULL, BAD_CAST name.c_str(),
				    BAD_CAST value.c_str()); 
	      
	    }
        }
        
        return newNode;
}

void TXMLAdapter::AddAttribute(TXMLNode node, std::string name, std::string value)
{
        xmlNewProp(node, BAD_CAST name.c_str(), BAD_CAST value.c_str());
}

void TXMLAdapter::WriteToFile(std::string filePath)
{
  
        xmlSaveFormatFileEnc(filePath.c_str(), m_document, "UTF-8", 1);
}


TXMLAdapter::~TXMLAdapter()
{
        xmlFreeDoc(m_document);
}

TXMLNode TXMLAdapter::RootNode()
{
        return (m_document == NULL)?NULL:xmlDocGetRootElement(m_document);
}

std::string TXMLAdapter::GetFirstNodeValue(std::string nodeName, TXMLNode parentNode)
{

        std::vector<string> names;
        std::string result;
        
        GetNodeValues(nodeName, parentNode, names);
        
        if(names.size() > 0){ result=names[0]; }
        else{ result=string(); }

        return result;
}

void TXMLAdapter::GetNodeValues(const std::string nodeName, TXMLNode parentNode, vector<string> &names)
{
        
        xmlChar *key;
        xmlNodePtr cur  = RootNode();
        cur = cur->children;
        while (cur != NULL) {
                 if ((xmlStrEqual(cur->name, (const xmlChar *) nodeName.c_str()))) {
                         key = xmlNodeListGetString(m_document, cur->xmlChildrenNode, 1);                         
                         if ( (const char*)key != NULL ){
                           names.push_back((const char*)key);
                         }
                         xmlFree(key);
                 }
                 cur = cur->next;
        }
        
        return;
}

std::vector<TXMLNode> TXMLAdapter::GetChildren(TXMLNode node, std::string name)
{
   xmlNodePtr child = node->children;
   if(!child)
           return vector<TXMLNode>();

   vector<TXMLNode> children;
   do
   {
           if(name.empty() || name == (const char*)child->name)
                   children.push_back(child);
   }
   while((child = child->next));
   
   return children;
}

void TXMLAdapter::Log() 
{
        m_currentNode = xmlDocGetRootElement(m_document);
        cout << m_currentNode->name << endl;
        m_currentNode = m_currentNode->xmlChildrenNode;
        while (m_currentNode != NULL) {
                cout << m_currentNode->name << endl;
        
                m_currentNode = m_currentNode->next;
        }
}




///Write Transformed XML 
void TXMLAdapter::WriteTransformedXML(std::string  sXMLFile, 
                                      map<std::string,std::string> m_Stylesheets)
{       

#ifdef XSL  
  
  //Pointer to stylsheet
  xsltStylesheetPtr cur = NULL;
  
  //input xml placeholder.
  xmlDocPtr doc;

    //iterate through the stylesheet list and apply them.
    std::map<std::string,std::string>::iterator sheet;


    for( sheet = m_Stylesheets.begin(); sheet != m_Stylesheets.end(); ++sheet)
      {
      //output file
      FILE* outfile;

      // Get source file and output file from Stylesheet map.
      //std::string sXSLTFile = sheet->first;

      if ( sheet->second != "none" ){

        //set oringinal xml input file to doc.
        doc = xmlParseFile(sXMLFile.c_str());

        // Open transform destination file 
        outfile = fopen(sheet->second.c_str(),"w");
      
        //get stylsheet into cur.    
        cur = xsltParseStylesheetFile((const xmlChar *)sheet->first.c_str());
        // Apply transform
        doc = xsltApplyStylesheet(cur, doc  , NULL);

        // Write out final result
        xsltSaveResultToFile( outfile, doc, cur);

        // Close file
        fclose(outfile);

        //free memory
        xsltFreeStylesheet(cur);
        xmlFreeDoc(doc);

      }

    }


  
#endif 
  
}
