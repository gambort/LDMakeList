#ifndef _TEXTSTUFF_H
#define _TEXTSTUFF_H

#include <string>
#include <vector>
using namespace std;

// Define if you want to convert tabs to space (slower)
#define LD_TAB_TO_SPC 1

//! Safe check of index - returns S[I] is I<S.length() or
//! char otherwise
#define SafeIndex(S,I,C) (char((I)<(S.length()))?(S[I]):(C))

// TEXT MANIPULATION

//! Strip space
inline string StripSpace(string S) {
  unsigned int i;
  int Leave;
  Leave=false;
  
#ifdef LD_TAB_TO_SPC
  // Replace tabs by spaces
  for (i=0; i<S.length(); i++) { if (S[i]=='\t') { S[i]==' '; } }
  // Find the first non-space
  for (i=0; ((i<S.length()) && (!Leave)); i++) {
    if (!(S[i]==' ')) { Leave=true; }
  }
#else
  // Find the first non-space/tab
  for (i=0; ((i<S.length()) && (!Leave)); i++) {
    if (!((S[i]==' ') || (S[i]=='\t'))) { Leave=true; }      
  }
#endif
  if (i>=1) { return S.substr(i-1); } else { return string(""); }
}


//! Strip space from both ends
inline string StripSpaceBoth(string S) {
  string R=StripSpace(S);
  int i, I=R.length();
  for (i=R.length()-1; i>=0; i--) {
    if ((R[i]==' ') || (R[i]=='\t')) {
      I--;
    } else {
      break;
    }
  }
  return R.substr(0,I);
}



//! Get the first word
inline string FirstWord(string S) {
  int isp;
  string T=StripSpace(S);

  if (T.length()==0) { return string(""); }
  if ((isp=T.find(' '))==string::npos) {
    return T;
  } else {
    if (isp>0) { return T.substr(0,isp); }
    else { return string(""); }
  }
}

//! Strip character from start if there
//! eg. StripChar("~Electric",'~') returns "Electric"
inline string StripChar(string S, char C) {
  if (S[0]==C) { return S.substr(1); } else { return S; }
}

//! Get all words after phrase eg.
//! S="0 BFC CERTIFY CCW"
//! P="BFC"
//! Returns "CERTIFY CCW"
//! If P not there simply return "";
inline string AllAfter(string S, string P) {
  string T=S;
  int ipos;
  if ((ipos=T.find(P))!=string::npos) {
    ipos+=P.length();
    if (ipos<S.length()) { return StripSpace(S.substr(ipos)); }
    else { return string(""); }
  } else {
    return string("");
  }
}

//! Like AllAfter, but before
inline string AllBefore(string S, string P) {
  string T=S;
  int ipos;
  if ((ipos=T.find(P))!=string::npos) {
    if (ipos>0) { return S.substr(0,ipos); }
    else { return string(""); }
  } else {
    return string("");
  }
}

//! Find the words between first occurences of _L_eft and _R_ight
//! eg. StringBetween("here [we] are", "[", "]") returns "we"
inline string StringBetween(string S, string L, string R) {
  int lpos, rpos;
  if ((lpos=S.find(L))!=string::npos) {
    if ((rpos=S.find(R,lpos))!=string::npos) {
      return S.substr(lpos+1, (rpos-lpos-1));
    }
  }
  return string("");
}

//! Make the string valid UTF8
inline int UTF8Fix(string& X) {
  char c;
  string R;
  int i;
  int err=-1;
  i=0;
  while (i<X.length()) {
    c=X[i];
    if (c&128) {
      // A crude check
      if ((c&224)==192) {
	if ((SafeIndex(X,i+1,255)&192)==128) {
	  i+=2;
	} else { X[i]='?'; i++; err=i; }
      } else if ((c&240)==224) {
	if (((SafeIndex(X,i+1,256)&192)==128)
	    && ((SafeIndex(X,i+2,256)&192)==128)) {
	  i+=3;
	} else { X[i]='?'; i++; err=i; }
      } else if ((c&248)==240) {
	if (((SafeIndex(X,i+1,256)&192)==128)
	    && ((SafeIndex(X,i+2,256)&192)==128)
	    && ((SafeIndex(X,i+3,256)&192)==128)) {
	  i+=4;
	} else { X[i]='?'; i++; err=i; }
      } else { X[i]='?'; i++; err=i; }
    } else {
      i++;
    }
  }
  return err;
}

//! Replace dangerous (to xml) characters from string by markup
inline string xmlString(string S) {
  int i;
  string X;
  for (i=0; i<S.length(); i++) {
    switch (S[i]) {
    case '<': X+="&lt;"; break;
    case '>': X+="&gt;"; break;
    case '&': X+="&amp;"; break;
    case '\'': X+="&apos;"; break;
    case '\"': X+="&quot;"; break;
    default:
      if (S[i]<128) { X+=S[i]; } else { printf("Weird char"); }
    }
  }
  return X;
}

//! Make an xml tag or attribute depending on what IsA(ttribute) says
//! If Att is defined it is a string for the attributes
inline string xmlTag(string Tag, string Value, bool IsA, string Att="") {
  string R;
  if (IsA) {
    // Pure attribute
    R=" "+xmlString(Tag)+"=\""+xmlString(Value)+"\" ";
  } else {
    if (Att.length()<=0) {
      //  No defined attributes
      if (Value.length()>0) {
	// With value
	R="<"+xmlString(Tag)+">"+xmlString(Value)+"</"+xmlString(Tag)+">";
      } else {
	// Without value
	R="<"+xmlString(Tag)+" />";
      }
    } else {
      // Defined attribute
      if (Value.length()>0) {
	// With value
	R="<"+xmlString(Tag)+" "+Att+
	  +" >"+xmlString(Value)+"</"+xmlString(Tag)+">";
      } else {
	// Without value
	R="<"+xmlString(Tag)+" "+Att+ " />";
      }
    }
  }
  return R;
}

//! If S is "" set to T else do S+=Split+T
inline void AddToEnd(string& S, string T, string Split) {
  if (S.length()==0) {
    S=T;
  } else {
    if (T.length()>0) { S+=Split+T; }
  }
}

//! Split into separated words on Sep
inline vector<string> SplitSep(string S, string Sep) {
  vector<string> R;
  string SR=S;
  int Pos;
  int SepLen=Sep.length();
  while ((Pos=SR.find(Sep))!=string::npos) {
    if ((Pos-SepLen) > 0) {
      R.push_back(SR.substr(0,(Pos-SepLen+1)));
    }
    SR=SR.substr(Pos+SepLen);
  }
  R.push_back(SR);
  return R;
}
//! Split into space separated words
inline vector<string> SplitSpace(string S) { return SplitSep(S, string(" ")); }


#endif
