/*! FILE: LDMakeList.C

  New version of Makelist for LDraw.
  Generates Parts.lst and Parts.xml from the LDraw parts library

  TODO:
  - Add support for .zip libraries
  - Add a time and date tag to the files (of file or of zip)
  - Allow it to read subparts?
 */

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

#include "CmdOpts.H"
#include "LDrawPart.H"

using namespace std;

#define VERSION "2.13"

// Windows-style directories (comment out for linux)
//#define WINSTYLE
#ifdef WINSTYLE
#warning Using windows style separators
#endif
// Number files per .
#define DCOUNT 100
// Old 64 character limit
#define MAXLEN 64
// Do we allow files called .ldr? (comment out if not)
// #define ALLOWLDR 1


void Info() {
  pf("LDMakeList\nUse -? for help, -v for version\n");
}

void Version() {
  pf("Version %s\n", VERSION);
  pf("Copyright 2012 Tim Gould\n");
  pf("Licensed under the GPL V3.0\n");
  pf("See gpl-3.0.txt for info or go to http://www.gnu.org/licenses/gpl-3.0.txt\n");
}

void Help() {
  pf("Valid options are:\n");
  pf("-d or -D : sort by _D_escription (default)\n");
  pf("-n or -N : sort by file_N_ame\n");
  //  pf("-c or -c : sort by _C_ategory then description\n");
  pf("-p or -P : sort by description (_P_atterns at end of file)\n");
  pf("-s or -S : use _S_hort descriptions*\n");
  pf("-m or -M : include pri_M_itives in the list\n");
  pf("-u or -U : include _U_nofficial part directory\n");
  pf("-a or -A : exclude _A_liases from the list\n");
  pf("-r or -R : _R_emove duplicate entries (experimental)\n");
  pf("-o or -O : Include _o_fficial parts only\n");
  pf("-l[DIRNAME] or -L[DIRNAME] : Read from _L_Draw director DIRNAME\n");
  pf("-h[C] or -H[C] : -H_ide descriptions starting with character(s) [C]\n");
  pf("-i[C] or -I[C] : str_I_p leading character(s) [C] from descriptions\n");
  pf("-x or -X : do not write parts._x_ml file\n");
  // pf("-z[Filename] or -Z[Filename] :\n");
  // pf("   Reads the parts from zipped parts directory in Filename (Experimental)\n");
  pf("\n");
  pf("eg. \"LDMakeList -d -h=_ -i~\" will generate a parts.lst file sorted\n");
  pf("by description (-d), excluding colour = and some parts _ (-h=_),\n");
  pf("and removing the ~ character (-i~) from the start of descriptions\n");
  pf("\n");
  pf("The -L tag lets you select the base directory for your LDraw\n");
  pf("installation (defaults to environment variable LDRAWDIR)\n");
  pf("eg. \"LDMakeList -d -L.\" sorts by descriptions in the present\n");
  pf("directory\n");
  pf("\n");
  pf("eg. \"LDMakeList -u\" will look in [LDRAWDIR]/Unofficial/Parts\n");
  pf("as well as [LDRAWDIR]/Parts for part files\n");
  pf("\n");
  pf("The -m tag is not recommended for beginner LDraw users\n");
  pf("\n");
  pf("* The old limit for part descriptions is 64 characters and it is\n");
  pf("possible that descriptions longer than this might break some\n");
  pf("old software. This ensures that the output filenames are no\n");
  pf("longer than 64 characters. Use if programs are giving errors.\n");
  //  pf("\n");
  //  pf("See LDListHelp.txt for more info!\n");
}


/* ***************************************************************** */
// LITTLE ROUTINES

//! Is it a .dat (or .ldr)?
int IsDatName(string S) {
  int L=S.length();
  const char *s=S.c_str();
  // Crude but hopefully pretty universal
#ifndef ALLOWLDR
  return ( ((s[L-3]=='d') || (s[L-3]=='D'))
	   && ((s[L-2]=='a') || (s[L-2]=='A'))
	   && ((s[L-1]=='t') || (s[L-1]=='T')));
#else
  return ( ( ((s[L-3]=='d') || (s[L-3]=='D'))
	     && ((s[L-2]=='a') || (s[L-2]=='A'))
	     && ((s[L-1]=='t') || (s[L-1]=='T'))) ||
	   ( ((s[L-3]=='l') || (s[L-3]=='L'))
	     && ((s[L-2]=='d') || (s[L-2]=='D'))
	     && ((s[L-1]=='r') || (s[L-1]=='R'))) );
#endif
}

// Master list of existing short names
vector<string> SNList;
// Make a short name of needed
int ShortName(string& Name, int ML) {
  if (Name.length()>ML) {
    int i,j;
    bool Match;
    string KName=Name.substr(0, ML-3);
    string TName;
    char No[10];
    // Keep incrementing until no match found
    for (i=0; i<1000; i++) {
      // Create short-hand for ith entry
      if (i==0) { sprintf(No,"..."); }
      else if (i<10) { sprintf(No,"..%d",i); }
      else if (i<100) { sprintf(No,".%2d",i); }
      else if (i<1000) { sprintf(No,"%3d",i); }
      else { er("Exceeded duplication limit (1000).\n"); exit(1); }
      TName=KName+No;
      // No match yet
      Match=false;
      for (j=0; j<SNList.size(); j++)
	if (TName==SNList[j]) { Match=true; continue; } // Crap, a match
      if (!Match) { i=1000; } // No match so we can leave
    }
    // Add to the list of existing names
    SNList.push_back(TName);
    Name=TName; // Return the shortened name
    return 1;
  } else return 0; // No need to change anything
}

//! Is the part a ~Moved to?
inline bool IsMoveTo(string S) {
  return !((S.substr(0,9).compare("~Moved to"))!=0);
}

//! Strip leading character if in strip list
inline void StripChar(Part& P, CmdOpts& Ctrl) {
  // Only if strip list contains elements
  if (Ctrl.NStrip>0) {
    char fc=P.Desc[0];
    int k;
    // Check and change
    for (k=0; k<Ctrl.NStrip; k++) {
      if (fc==Ctrl.StripList[k]) {
	P.Desc=P.Desc.substr(1,P.Desc.length()-1);
      }
    }
  }
}

/* ***************************************************************** */
// MAIN ROUTINES

//! Get top level Directory name
string GetTopDirName(CmdOpts Ctrl) {
  string LDir;
  char *ldir;

  if (Ctrl.UseZip) {
    LDir=Ctrl.ZipFileName;
  } else {
    // Set the directory
    if (Ctrl.LDrawDir[0]==0) {
      ldir=getenv("LDRAWDIR");
      if (ldir==NULL) {
	LDir=".";
	//	er("Environment variable LDRAWDIR is not defined.\nEither specify or use -lLDRAWDIR option\n"); exit(1);
      } else { LDir=ldir; }
    } else {
      LDir=Ctrl.LDrawDir;
    }
  }
  return LDir;
}

//! Get a list of filenames
vector<string> GetDirList(CmdOpts Ctrl, string DName, string SDName,
			  string& DirName) {
  string FileName;
  string FilePath;

  vector<string> FileList;

  if (Ctrl.UseZip) {
    // Zip file
    er("Reading from .zip file %s (Experimental feature - not working)\n",
       Ctrl.ZipFileName.c_str());
    exit(1);
  } else {
    // Check through directory
    DIR *dir;
    struct dirent *dirEntry;
    struct stat filestat;
    string LDir;

    LDir=GetTopDirName(Ctrl);

#ifdef WINSTYLE
    DirName=LDir+"\\"+DName+"\\";
    DName=DirName+SDName+"\\";
#else
    DirName=LDir+"/"+DName+"/";
    DName=DirName+SDName+"/";
#endif
    
    pf("Reading from %s\n", DName.c_str());

    // Open it
    dir=opendir(DName.c_str());
    // Fail if needed
    if (dir==NULL) {
      er("Parts directory %s could not be opened\n", DName.c_str());
    } else {
      while ((dirEntry=readdir(dir))) {
	if (SDName.length()>0) {
#ifdef WINSTYLE
	  FileName = SDName+"\\"+dirEntry->d_name;
#else
	  FileName = SDName+"/"+dirEntry->d_name;
#endif
	} else {
	  FileName = dirEntry->d_name;
	}
	FilePath = DirName + FileName;
	
	// If the file is a directory (or is in some way invalid) we'll skip it 
	if (stat( FilePath.c_str(), &filestat )) continue;
	if (S_ISDIR( filestat.st_mode ))         continue;
	
	FileList.push_back(FileName);
      }
    }
  }

  return FileList;
}

//! Read the parts directory and return a part list
//! Optional entry set to true for primitives directory
vector<Part> ReadPartDir(CmdOpts Ctrl, string DName, string SDName,
			 int IsPrimitive=0) {
  int i,h;
  string FileName;
  string DirName;
  string DirNameText;
  string FilePath;
  string DDName;

  Part tpart;
  vector<Part> PList;

  char fc;
  bool Ignore;

  vector<string> FileList=GetDirList(Ctrl, DName, SDName, DirName);
  int iFL;

  FileIO File;

  // Read through the directory
  int NR=0, ND=0, NH=0, NU=0, NA=0;
  pf("Reading directory (.=%d files):\n", DCOUNT);
  for (iFL=0; iFL<FileList.size(); iFL++) {
    FileName = FileList[iFL];

    // If it's a DAT, do stuff
    if (IsDatName(FileName)) {
      if (!Ctrl.UseZip) {
	// Open the file
	FilePath = DirName + "/" + FileName;
	if (!File.Open(FilePath)) {
	  er("Cannot open %s\n", FileName.c_str()); exit(1);
	}
      } else {
	// Next file in the .zip
	er("Zip support not implemented\n"); exit(1);
      }

      DirNameText=DirName;
      if (Ctrl.LDrawDir[0]==0) {
#ifdef WINSTYLE
	DirNameText=string("%LDRAWDIR%\\")+DName+"\\";
#else
	DirNameText=string("%LDRAWDIR%/")+DName+"/";
#endif
      }      

      // Get the part information     
      tpart=GetPartInfo(File, FileName, DirNameText);
      tpart.Primitive=IsPrimitive;
      // Close the file
      File.Close();
      // If it's got non-zero length
      if (tpart.Desc.length()>0) {
	// Read a file
	NR++;
	// Get the first character of the description
	fc=tpart.Desc[0]; 
	// Only ignore if needed
	Ignore=false;
	// Ignore unofficial files if asked
	if ((Ctrl.RemoveUn) && (tpart.Unofficial)) { NH++; Ignore=true; }
	// Ignore alias parts if asked
	if ((Ctrl.ExcludeAlias) && (tpart.Alias)) { NH++; NA++; Ignore=true; }
	// Ignore a moved to
	if ((fc=='~') && (IsMoveTo(tpart.Desc))) { NH++; Ignore=true; }
	// Ignore anything from the user specified list
	for (h=0; h<Ctrl.NHide; h++)
	  if (fc==Ctrl.HideList[h]) { NH++; Ignore=true; }
	
	// If not ignored, add the part
	if (!Ignore) {
	  // Add to unofficials list
	  if (tpart.Unofficial) { NU++; }
	  // Add the part
	  StripChar(tpart,Ctrl);
	  PList.push_back(tpart);
	  ND++;
	} else {
	  // Or not
	  //pf("Ignoring %s\n", tpart.Desc.c_str());
	}
      }
      // Keep a count
      if (((NR+1)%DCOUNT)==0) {	
	printf(".");
      }
    }
  }
  pf("\nNumber parts read:    %6d\n", NR);
  if (NU>0) { pf("Number unoffs read:   %6d\n", NU); }
  if (NH>0) { pf("Number parts ignored: %6d\n", NH); }
  if (NA>0) { pf("Number alias ignored: %6d\n", NA); }

  return PList;
}

//! Read a family of parts (official and unofficial) from DIR/SDIR,
//! set the part tag to Type and add the parts to PList
void ReadFamily(vector<Part>& PList, CmdOpts Ctrl,
		string Dir, string SDir, int IsPrimitive=0) {
  unsigned int i;
  vector<Part> PP;

  PP=ReadPartDir(Ctrl, Dir, SDir, IsPrimitive);
  for (i=0; i<PP.size(); i++) { PList.push_back(PP[i]); }
  if (Ctrl.IncludeUn) {
#ifdef WINSTYLE
    PP=ReadPartDir(Ctrl, string("UNOFFICIAL\\")+Dir, SDir, IsPrimitive);
#else
    PP=ReadPartDir(Ctrl, string("UNOFFICIAL/")+Dir, SDir, IsPrimitive);
#endif
    for (i=0; i<PP.size(); i++) { PList.push_back(PP[i]); }
  }
}

//! Read part (and prims if needed) directories
vector<Part> ReadDirs(CmdOpts Ctrl) {
  vector<Part> PList;
  string SN="";


  unsigned int i;
  vector<Part> PP;
  // Include unofficial parts
  ReadFamily(PList, Ctrl, string("Parts"), SN);
  // Include primitives
  if (Ctrl.IncludePrims) {
    ReadFamily(PList, Ctrl, string("P"), SN, LD_PRIMITIVE);
    ReadFamily(PList, Ctrl, string("P"), string("0"), LD_0PRIMITIVE);
    ReadFamily(PList, Ctrl, string("P"), string("8"), LD_8PRIMITIVE);
    ReadFamily(PList, Ctrl, string("P"), string("48"), LD_48PRIMITIVE);
  }
  
  return PList;
}

//! Main
int main(int argn, char **argv) {
  CmdOpts Ctrl;
  string tstr;
  int i, j, k;

  char tcstr[256];
  Part tpart;
  vector<Part> PList;
  vector<Part>::iterator p;

  // Process the arguments
  Ctrl.ProcArgs(argn, argv);
  
  // Read the parts directory
  PList=ReadDirs(Ctrl);

  // No entries is a fail
  if (PList.size()==0) { er("No entries to read\n"); exit(1); }

  // Sort the part list
  if (Ctrl.SortType==BYNUMBER) {
    pf("Sorting by number\n");
    if (Ctrl.RemoveDups) {
      sort(PList.begin(), PList.end(), CmpPartDesc);
      RemoveDups(PList);
    }
    sort(PList.begin(), PList.end(), CmpPartNumber);
  } else if (Ctrl.SortType==BYDESCPATTERN) {
    pf("Sorting by description (patterns last)\n");
    sort(PList.begin(), PList.end(), CmpPartDescPattern);
    if (Ctrl.RemoveDups) { RemoveDups(PList); }
  } else if (Ctrl.SortType==BYCATDESC) {
    pf("Sorting by category, then description\n");
    sort(PList.begin(), PList.end(), CmpPartCatDesc);
    if (Ctrl.RemoveDups) { RemoveDups(PList); }
  } else {
    pf("Sorting by description\n");
    sort(PList.begin(), PList.end(), CmpPartDesc);
    if (Ctrl.RemoveDups) { RemoveDups(PList); }
  }


  // Output the part list
  FILE *fil,*xfil;
  string Desc;
  string xmlstr;
  int UTF8err;

  // Open and setup the files
  fil=fopen("parts.lst","w");
  if (!Ctrl.Noxml) {
    xfil=fopen("parts.xml","w"); // Make an xml file too
    fprintf(xfil, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

    fprintf(xfil, "%s\n", xmlHeader().c_str());
    fprintf(xfil, "%s\n", xmlTopDir(GetTopDirName(Ctrl)).c_str());

    fprintf(xfil, "<OS-Properties ");
#ifdef WINSTYLE
    fprintf(xfil, "Style=\"DOS\"");
#else
    fprintf(xfil, "Style=\"POSIX\"");
#endif
    fprintf(xfil, " />\n\n");
  }
  int SN=0;
  for (i=0; i<PList.size(); i++) {
    tpart=PList[i];
    Desc=tpart.Desc;
    if (tpart.Primitive==LD_PRIMITIVE) { Desc="Primitive : "+Desc; }
    if (tpart.Primitive==LD_48PRIMITIVE) { Desc="Hi-res primitive : "+Desc; }
    if (tpart.Primitive==LD_0PRIMITIVE) { Desc="Lo-res primitive : "+Desc; }
    if (tpart.Primitive==LD_8PRIMITIVE) { Desc="Very lo-res primitive : "+Desc; }
    // Shorten description if needed
    if (Ctrl.ShortDescs) {
      SN+=ShortName(Desc, MAXLEN);
    }
    // Output the details
    fprintf(fil, "%-27s%s\n", tpart.Number.c_str(), Desc.c_str() );
    if (!Ctrl.Noxml) {
      xmlstr=tpart.xmlOut();
      if ((UTF8err=UTF8Fix(xmlstr))>0) {
	printf("Warning! File %s contains non-UTF-8 characters!\n",
	       tpart.Number.c_str());
      }      
      fprintf(xfil, "%s\n", xmlstr.c_str());
    }
  }
  // And finish off the files
  if (!Ctrl.Noxml) {
    fprintf(xfil, "%s\n", xmlFooter().c_str());
    fclose(xfil);
  }
  fclose(fil);
  if (SN>0) { pf("Shortened names:      %6d\n", SN); }
  pf("Number parts written: %6d\n", PList.size());
  if (!Ctrl.Noxml) {
    pf("\"part.lst\" and \"parts.xml\" written to current directory\n");
  } else { pf("\"part.lst\" written to current directory\n"); }
}


