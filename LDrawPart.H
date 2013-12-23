#ifndef _LDRAWPART_H
#define _LDRAWPART_H

#include<stdio.h>

#include<cstdlib>
#include<iostream>
#include<sstream>
#include<vector>
#include<algorithm>
#include<string>

#include "TextStuff.H"
#include "FileIO.H"

/* CHANGES:
   V2.0 XML

   - Add <OS-Properties> BEFORE <LDraw-Library>
   - Added LDRAWDIR tag at start of <LDraw-Library>, after <Version>
   - Split author into real author name and username
   - Keywords tag removed
   - License now shows
 */

/*
  TODO:
 */

using namespace std;

// Uncomment to show the license
#define LD_SHOW_LICENSE
// Uncomment to show the comments
//#define LD_SHOW_COMMENTS
// Uncomment to show warnings
//#define LD_WARNINGS


// Use sub-tags
/*
#define LD_XML_STARTTAG "<FileEntry"
#define LD_XML_MIDTAG ">\n"
#define LD_XML_ENDATT ""
#define LD_XML_ENDTAG "</FileEntry>"
#define LD_XML_ATTRIB false
*/
// Use attributes
#define LD_XML_STARTTAG "<FileEntry"
#define LD_XML_MIDTAG ""
#define LD_XML_ENDATT " >\n"
#define LD_XML_ENDTAG "</FileEntry>"
#define LD_XML_ATTRIB true

// Types
#define LD_UNKNOWN 0
#define LD_PART 1
#define LD_SHORTCUT 2
#define LD_PART_PHYSICAL_COLOUR 3
#define LD_SUBPART 4
#define LD_PRIMITIVE 5
#define LD_48PRIMITIVE 6
#define LD_8PRIMITIVE 7
#define LD_0PRIMITIVE 8

#define LD_TYPEMASK 15
// Qualifiers must use latter four bits
// #define LD_QUAL 16
#define LD_QUALMASK 240



//! xml header and footer
inline string xmlHeader() {
  return string("<LDraw-Library>\n")
    + string("<Version Number=\"2\" />");
}
inline string xmlTopDir(string LDir) {
  return xmlTag("LDRAWDIR", LDir, false)+"\n";
}
inline string xmlFooter() {
  return string("</LDraw-Library>");
}

//! A part has a Number and Desc(ription)
//! But it may have other features too eg. Pattern
class Part {
 public:
  string Number;
  string Desc;
  string Author;
  bool Pattern;
  bool Unofficial;
  bool Alias;
  bool PColour;
  int Primitive;
  int DATType;
  string PathName;

  string LDrawOrg;
  string License;
  string Help;
  string History;
  string Category;
  string Keywords;
  string BFC;
  string Comments;

  Part() {
    Number="";
    Desc="";
    Author="";
    Pattern=false;
    Unofficial=false;
    Alias=false;
    PColour=false;
    Primitive=0;
    PathName="";

    LDrawOrg="";
    License="";
    Help="";
    History="";
    Category="";
    Keywords="";
    BFC="";
    Comments="";
  }
  
  // Return an xml formatted line
  string xmlOut() {
    string X;
    string TS, TS2, TS3;
    vector<string> VS;
    int iS;
    string NL="\n";

    // Define the entry
    X=LD_XML_STARTTAG;
    // What type and qualifier
    VS=SplitSpace(LDrawOrg);
    if (VS.size()>=1) {
      if (VS[0].length()>0) { X+=xmlTag("Filetype", VS[0], true); }
    } else {
      if (VS[0].length()>0) { X+=xmlTag("Filetype", VS[0], true); }
      /*
      if (VS[1]=="Alias") { X+=xmlTag("IsAlias", "True", true); }
      else if (VS[1]=="Physical_Colour") {
	X+=xmlTag("IsPhsyicalColour", "True", true); }
      */
    }
    if (Alias) { X+=xmlTag("IsAlias", "True", true); }
    if (PColour) { X+=xmlTag("IsPhysicalColour", "True", true); }
    // Is it official
    if (Unofficial) { X+=xmlTag("IsOfficial", "False", true)+NL; }
    else { X+=xmlTag("IsOfficial", "True", true)+NL; }
    // End the bit that must be attributes
    X+=LD_XML_MIDTAG; // End the entry

    X+=xmlTag("NameEntry", Number, LD_XML_ATTRIB)+NL;

    X+=xmlTag("Description", Desc, LD_XML_ATTRIB)+NL;

    TS2=StringBetween(Author,"[","]");
    if (TS2.length()==0) {    
      X+=xmlTag("Author", Author, LD_XML_ATTRIB);
      X+=xmlTag("Username", "", LD_XML_ATTRIB)+NL;
    } else {
      TS=StripSpaceBoth(Author.substr(0,Author.find("[")));
      X+=xmlTag("Author", TS, LD_XML_ATTRIB);
      X+=xmlTag("Username", TS2, LD_XML_ATTRIB)+NL;      
    }

    if (Category.length()>0)
      { X+=xmlTag("Category", Category, LD_XML_ATTRIB)+NL; }
    else
      { X+=xmlTag("Category", StripChar(FirstWord(Desc),'~'),
		  LD_XML_ATTRIB)+NL; }

#ifdef LD_SHOW_LICENSE
    if (License.length()>0)
      { X+=xmlTag("License", License, LD_XML_ATTRIB)+NL; }
#endif

    X+=xmlTag("FilenameWithPath", PathName, LD_XML_ATTRIB)+NL;

    // End the bit that can be attributes
    X+=LD_XML_ENDATT;

    // Split and write the keywords
    if (Keywords.length()>0) {      
      VS=SplitSep(Keywords,",");
      for (iS=0; iS<VS.size(); iS++) {
	TS=StripSpace(VS[iS]);
	X+=xmlTag("Keyword", TS, false);
      }
      X+=NL;
    }
    // Split and write the BFC
    if (BFC.length()>0) {
      TS2="False";
      VS=SplitSpace(BFC);
      TS=StripSpace(VS[0]);
      if (TS=="CERTIFY") {
	if (VS.size()==1) { TS="CCW"; }
	else { TS=xmlString(StripSpace(VS[1])); }
	TS2="True";
      } else if (TS=="CLIP") {
	TS2="False";
	if (VS.size()==1) { TS="CCW"; }
	else { TS=xmlString(StripSpace(VS[1])); }
      } else if ((TS=="CW") || (TS=="CCW")) {
	TS2="False";
      }
      string Att;
      Att="Certify=\""+TS2+"\"";
      if ((TS.length()>0) && (TS.find("INVERT")==string::npos))
	{ Att+=" Winding=\""+TS+"\""; }      
      X+=xmlTag("BFC", "", false, Att)+NL;
    }
    // Split and write the history
    if (History.length()>0) {
      // Split up via newlines
      VS=SplitSep(History, "\n");
      // TODO : Process {multi word} author tags
      for (iS=0; iS<VS.size(); iS++) {
	bool C=false;
	// The first word is always the date
	TS=FirstWord(VS[iS]);
	// Look for a regular username in [UserName]
	TS2=StringBetween(VS[iS],"[","]");
	if (TS2.length()>0) {
	  // Found it - rest of the line is the actual history commant
	  TS3=StripSpace(AllAfter(VS[iS],TS2+"]"));
	} else {
	  // No [UserName], try {UserName from elsewhere}
	  TS2=StringBetween(VS[iS],"{","}");
	  if (TS2.length()>0) {
	    // Found it - rest of the line is the actual history commant
	    C=true;
	    TS3=StripSpace(AllAfter(VS[iS],TS2+"}"));
	  } else {
	    // No valid username, assume "DATE Other history stuff"
	    TS2="";
	    TS3=StripSpace(AllAfter(VS[iS],TS));
	  }
	}

	string Attribs="Date=\""+TS+"\" Username=\""+TS2+"\"";
	if (C) Attribs+=" External=\"True\"";
	X+=xmlTag("History", TS3, false, Attribs)+NL;
      }
    }
    // Write the help and comments
    if (Help.length()>0)
      { X+=xmlTag("Help", Help, false)+NL; }
#ifdef LD_SHOW_COMMENTS
    if (Comments.length()>0)
      { X+=xmlTag("Comments", Comments, false)+NL; }
#endif
    // End the entry
    X+=LD_XML_ENDTAG+NL;
    return X;
  }
};


//! Compare by Number (or Desc if Number same)
inline int CmpPartNumber(const Part P1, const Part P2) {
  if (P1.Number==P2.Number) { return (P1.Desc<P2.Desc); }
  else { return (P1.Number<P2.Number); }
}

//! Compare by Desc (or Number if Desc same)
inline int CmpPartDesc(const Part P1, const Part P2) {
  if (P1.Desc==P2.Desc) { return (P1.Number<P2.Number); }
  else { return (P1.Desc<P2.Desc); }
}

//! Compare by Desc (or Number if Desc same) - Patterns last
inline int CmpPartDescPattern(const Part P1, const Part P2) {
  if ((P1.Pattern) && !(P2.Pattern)) {
    return false;
  } else if (!(P1.Pattern) && (P2.Pattern)) {
    return true;
  } else {
    if (P1.Desc==P2.Desc) { return (P1.Number<P2.Number); }
    else { return (P1.Desc<P2.Desc); }
  }
}

//! Compare by categoty (then Desc, then Number)
inline int CmpPartCatDesc(const Part P1, const Part P2) {
  if (P1.Category==P2.Category) {
    return CmpPartDesc(P1, P2);
  } else { return P1.Category<P2.Category; }
}


//! Are the descriptions the same?
inline int EqPartDesc(const Part P1, const Part P2) {
  return (P1.Desc==P2.Desc);
}

//! Remove duplicate entries (crude)
extern void RemoveDups(vector<Part>& PL);

//! Get the part info, ignoring leading space
extern Part GetPartInfo(FileIO& File, string FileName, string DirName);

#endif
