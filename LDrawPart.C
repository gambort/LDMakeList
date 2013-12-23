#include "LDrawPart.H"

// Maximum allowed header length
#define LD_MAX_HDR_LENGTH 50

// LDRAW MANIPULATION

//! Get the line type (0,1,2,3,4,5) and its data
//! Return Type=-1 if none of the above
inline string GetLineType(string S, int &Type) {
  string t=StripSpace(S);
  if (t.length()>0) {
    char ct=t[0];
    
    if ((ct=='0') || (ct=='1') || (ct=='2')
	|| (ct=='3') || (ct=='4') || (ct=='5')) {
      Type=ct-48; // Convert from character to number
      t=StripSpace(t.substr(1));
      return t;
    } else { Type=-1; return string(""); }
  } else { Type=-1; return string(""); }
}

//! Meta and data
inline string GetMeta(string S, string& Meta) {
  int i;
  int iex;
  int isp;
  // Does it have !
  if ((iex=S.find('!'))!=string::npos) {
    // find first following ' '
    isp=S.find(' ', iex);
    Meta=S.substr(iex+1,(isp-iex-1));
    return StripSpace(S.substr(isp+1));
  } else {
    Meta="";
    return "";
  }
}

//! Get the part info
Part GetPartInfo(FileIO& File, string FileName, string DirName) {
  string Meta, Value;
  int lti;
  string L, LT;
  Part P;


  P.Number=FileName;
  if ((P.Number.find('P',0)!=string::npos)
      || (P.Number.find('p',0)!=string::npos)) {
    P.Pattern=true;
  } else { P.Pattern=false; }

  P.PathName=DirName+FileName;

  P.Unofficial=false;
  
  // Get the description
  L=StripSpace(File.ReadLine());
  if (L.length()>0) {
    P.Desc=GetLineType(L, lti);
  } else {
    printf("Part %s has no description\n", P.Number.c_str());
  }
  // Ignore name
  L=File.ReadLine();
  // Get Author
  L=GetLineType(File.ReadLine(), lti);
  if (lti==0) {
    Value=AllAfter(L, "Author:");
    P.Author=Value;
  }
  
  // And find LDraw_Org info
  L=File.ReadLine();
  // Crude unofficial check
  if (L.find(string("Unofficial"),0)!=string::npos) {
    P.Unofficial=true;
  }
  // Get part type
  Value=GetMeta(L, Meta);
  if (Meta!="LDRAW_ORG") {
    if (L.find("LDRAW_ORG")!=string::npos) {
#ifdef LD_WARNINGS
      printf("Warning: Part %s has LDRAW_ORG not !LDRAW_ORG\n",
	     P.Number.c_str());
#endif
      P.LDrawOrg=AllAfter(L, "LDRAW_ORG");
    } else {
      P.Unofficial=true;
      P.LDrawOrg="Undefined";    
    }
  } else {
    P.LDrawOrg=Value;
  }
  string FW=FirstWord(P.LDrawOrg);
  if (FW=="Part") {
    P.DATType=LD_PART;
    // Check if it's an alias (crude)
    if ((P.LDrawOrg.find("Alias")!=string::npos)
	|| (P.LDrawOrg.find("alias")!=string::npos)) {
      P.Alias=true;
    }
    if ((P.LDrawOrg.find("Physical_Colour")!=string::npos)
	|| (P.LDrawOrg.find("Physical_Color")!=string::npos)) {
      P.PColour=true;
    }
  }
  else if (FW=="Subpart") { P.DATType=LD_SUBPART; }
  else if (FW=="Primitive") { P.DATType=LD_PRIMITIVE; }
  else if (FW=="0_Primitive") { P.DATType=LD_0PRIMITIVE; }
  else if (FW=="8_Primitive") { P.DATType=LD_8PRIMITIVE; }
  else if (FW=="48_Primitive") { P.DATType=LD_48PRIMITIVE; }
  else if (FW=="Shortcut") { P.DATType=LD_SHORTCUT; }
  else { P.DATType=LD_UNKNOWN; }
  
  // Get the remainder of the unordered header
  bool KeepGoing=true, EndHeader=false;
  int iH=0;
  while ( KeepGoing ) {
    // Terminate if not a comment or blank line, or reached end of file
    L=GetLineType(File.ReadLine(), lti);
    if (lti==0) {
      // Line is a comment - do something
      
      Value=GetMeta(L, Meta);
      if (Meta.length()>0) {
	if (Meta==string("LICENSE")) { P.License=Value; }
	if (Meta==string("CATEGORY")) { P.Category=Value; }
	if (Meta==string("KEYWORDS")) {
	  AddToEnd(P.Keywords, Value, string(", "));
	}
	if (Meta==string("HELP")) {
	  AddToEnd(P.Help, Value, string("\n"));
	}
	if (Meta==string("HISTORY")) {
	  AddToEnd(P.History, Value, string("\n"));
	}
	if (Meta==string("ENDOFHEADER")) { EndHeader=true; }
      } else {
	// Comment but not a keyword
	Value=AllAfter(L, string("BFC"));
	if (Value.length()>0) { P.BFC=Value; }

	Value=AllAfter(L, string("//"));
	AddToEnd(P.Comments, Value, string("\n"));
      }    
    }
    
    iH++;
    if (iH>LD_MAX_HDR_LENGTH) {
      printf("Uh oh - part %s has a very long header\n", P.Number.c_str());
      exit(1);
    }

    KeepGoing=((lti<1) && (!File.EndOF()) && (!EndHeader) );
  }
  
  return P;
}

//! Remove duplicates
void RemoveDups(vector<Part>& PL) {
  unsigned int i;
  vector<unsigned int> Rmv;
  // List is sorted so check neighbours i, i+1
  for (i=0; i<(PL.size()-1); i++) {
    if (EqPartDesc(PL[i],PL[i+1] )) {
      // If same add i to the removal list
      Rmv.push_back(i);
    }
  }
  // Remove elements in list
  int s=0; // To deal with changing vector size
  for (i=0; i<Rmv.size(); i++) {
    PL.erase(PL.begin()+Rmv[i]-s);
    s++; // One element out so one element fewer before next i
  }
  if (Rmv.size()>0) { printf("Number parts removed: %6d\n", Rmv.size()); }
}
