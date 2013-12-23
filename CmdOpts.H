#ifndef _CMDOPTS_H
#define _CMDOPTS_H

#include <string>


using namespace std;

extern void Info();
extern void Help();
extern void Version();

// Short-hand for better legibility
#define pf printf
#define wa printf("WARNING: "); printf
#define er printf("ERROR: "); printf

// Maximum elements allowed in lists
#define MAXLIST 512

// Poor mans enum
#define BYDESC 0
#define BYNUMBER 1
#define BYDESCFANCY 2
#define BYDESCPATTERN 3
#define BYCATDESC 4

class CmdOpts {
public:
  //! Type of sort
  int SortType;
  //! Use short descriptions
  bool ShortDescs;
  //! Exclude alias parts
  bool ExcludeAlias;
  //! Include unofficial parts
  bool IncludeUn;
  //! Include primitives
  bool IncludePrims;
  //! Remove duplicates
  bool RemoveDups;
  //! Remove unofficial parts
  bool RemoveUn;
  //! LDrawDir name
  char LDrawDir[MAXLIST]; 
  //! List of characters to be hidden
  char HideList[MAXLIST];
  char NHide;
  //! List of characters to be stripped
  char StripList[MAXLIST];
  char NStrip;
  //! Read from a zip
  bool UseZip;
  //! Zip file name
  string ZipFileName;
  //! Do not write xml file
  bool Noxml;

  void ProcArgs(int argn, char**argv) {
    int i,k;
    SortType=BYDESC;
    ExcludeAlias=false;
    IncludeUn=false;
    IncludePrims=false;
    ShortDescs=false;
    RemoveDups=false;
    RemoveUn=false;
    LDrawDir[0]=0;
    NHide=0;
    NStrip=0;
    UseZip=false;
    Noxml=false;

    Info();
    // No arguments, query (as per old mklist)
    if (argn==1) {
      Help();
      pf("Sort by [D]escription or [N]umber?\n");
      char c=fgetc(stdin);
      if ((c=='n') || (c=='N')) {
	SortType=BYNUMBER; // By number
      }
    }
    // Go through the list
    for (i=1; i<argn; i++) {
      // Is it a command?
      if (argv[i][0]=='-') {
	// Choose your poison
	switch (argv[i][1]) {
	case '?':
	  Help();
	  exit(0);
	  break;
	case 'v': case 'V':
	  Version();
	  exit(0);
	  break;
	case 'd': case 'D':
	  SortType=BYDESC; // By description
	  break;
	case 'p': case 'P':
	  SortType=BYDESCPATTERN; // By description (patterns last)
	  break;
	case 'n': case 'N':
	  SortType=BYNUMBER; // By number
	  break;
	  //case 'c': case 'C':
	  //SortType=BYCATDESC; // By number
	  //break;
	case 's': case 'S':
	  ShortDescs=true; // Shorten descriptions
	  break;
	case 'a': case 'A':
	  // Exclude alias parts
	  ExcludeAlias=true;
	  break;
	case 'u': case 'U':
	  // Include unofficial parts
	  IncludeUn=true;
	  break;
	case 'm': case 'M':
	  // Include primitives
	  IncludePrims=true;
	  break;
	case 'r': case 'R':
	  RemoveDups=true; // Remove duplicates
	  break;
	case 'o': case 'O':
	  RemoveUn=true; // Remove unofficial parts
	  break;
	case 'l': case 'L':
	  k=2;
	  while (argv[i][k]!=0) {
	    LDrawDir[NHide]=argv[i][k];
	    NHide++;
	    if (NHide>=MAXLIST) {
	      er("Directory name too long\n"); exit(1);
	    }
	    k++;
	  }
	  break;
	case 'h': case 'H':
	  // Make a list of leading characters to ignore
	  k=2;
	  while (argv[i][k]!=0) {
	    HideList[NHide]=argv[i][k];
	    NHide++;
	    if (NHide>=MAXLIST) {
	      er("Too many elements in hide list\n"); exit(1);
	    }
	    k++;
	  }
	  if (NHide==0) {
	    wa("Nothing added to hide list - perhaps you wanted \"-?\" help?\n");
	  }
	  break;
	case 'i': case 'I':
	  // Make a list of leading characters to ignore
	  k=2;
	  while (argv[i][k]!=0) {
	    StripList[NHide]=argv[i][k];
	    NStrip++;
	    if (NStrip>=MAXLIST) {
	      er("Too many elements in strip list\n"); exit(1);
	    }
	    k++;
	  }
	  break;
	case 'x': case 'X':
	  Noxml=true;
	  break;
	case 'z': case 'Z':
	  // Read from zip - get the Filename
	  if (argv[i][2]==0) {
	    er("Must specifify a filename after -z\n"); exit(1);
	  }
	  UseZip=true;
	  ZipFileName="";
	  k=2;
	  while (argv[i][k]!=0) {
	    ZipFileName+=argv[i][k];
	    k++;
	  }
	  break;
	}
      }
    }
  };
};

#endif
