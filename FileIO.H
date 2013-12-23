#ifndef _FILEIO_H
#define _FILEIO_H

/*! FILE: FileIO.H
  
  This file separates out the data reading routine, to
  allow access to the GetLine function which reads a
  given line until some combination of \10 and/or \13 is
  found.

  Presently it can only read a 'physical' file from the
  file system, but by replacing the "Open", "Close"
  and "Get(Temp)Stream" functions we can allow it to
  directly access other streams e.g. compressed file data
 */

#include <stdio.h>
#include <string>

using namespace std;

// Read 16k at a time
#define FIO_CHUNK 16384
// Temporary chunk
#define FIO_TEMPCHUNK 16

#define FIO_NONE 0
#define FIO_DATASTREAM 1
#define FIO_FILESTREAM 2
#define FIO_FIO_CHUNKSSTREAM 4

class FileIO {
 public:
  //! Default constructor
  FileIO() {
    WhereInStream=0;
    LengthStream=0;
    Stream=NULL;
    TempStream=NULL;
    FilePtr=NULL;
  }
  ~FileIO() {
    Close();
  }

  //! Open a file from a filename
  bool Open(string FN) {
    FileStyle=FIO_FILESTREAM;

    FileName=FN;
    FilePtr=fopen(FileName.c_str(), "rb");
    EndOfFile=false;
    
    Stream=new char[FIO_CHUNK];
    TempStream=new char[FIO_TEMPCHUNK];
    GetStream();

    return (FilePtr!=NULL);
  };

  //! Open a data stream
  bool Open(char* DataPtr, long int Size) {
    FileStyle=FIO_DATASTREAM;

    LastStream=true;
    EndOfFile=false;
    
    Stream=DataPtr;
    WhereInStream=0;
    LengthStream=Size;
    TempStream=new char[FIO_TEMPCHUNK];

    return true;;
  };

  //! Close the FileIO
  void Close() {
    LengthStream=0;
    WhereInStream=0;
    // So long as it's not a FIO_DATASTREAM do some clean up
    if (!(FileStyle==FIO_DATASTREAM)) {
      if (TempStream!=NULL) { delete [] TempStream; TempStream=NULL; }
      if (Stream!=NULL) { delete [] Stream; Stream=NULL; }
      
      if (FileStyle==FIO_FILESTREAM) {
	if (FilePtr!=NULL) { fclose(FilePtr); FilePtr=NULL; }
      }
      FileName="";
    }
    FileStyle=FIO_DATASTREAM;
  }

  //! Read a line terminated by any combination of \10 and \13
  string ReadLine() {
    char c, tc;
    bool Done;
    string L;
    
    // Read the line
    Done=false;
    while ( !Done ) {
      c=GetChar();
      if ((c=='\10') || (c=='\13') || (c=='\n') || (c=='\r')) {
	Done=true;
	tc=GetTempChar();
	if ((tc=='\10') || (tc=='\13') || (tc=='\n') || (tc=='\r')) {
	  c=GetChar();
	}
      } else {
	L=L+c;
      }
      if (EndOfFile) { Done=true; }
    }

    //printf("[%s]\n", L.c_str());
    return L;
  }

  //! At end of file?
  bool EndOF() { return EndOfFile; }

 private:
  unsigned int FileStyle;

  FILE* FilePtr;
  string FileName;
  bool EndOfFile;

  char* Stream;
  char* TempStream;
  unsigned long int LengthStream;
  unsigned long int WhereInStream;
  bool LastStream;

  //! Get a stream of FIO_CHUNK characters
  void GetStream() {
    if (FileStyle==FIO_DATASTREAM) {
      // A datastream does not chunk and is always Last
      LastStream=true;
    } else {
      LastStream=false;
      
      // Read the stream
      if (FileStyle==FIO_FILESTREAM) {
	LengthStream=fread(Stream, sizeof(char), FIO_CHUNK, FilePtr);
	if (ferror(FilePtr)) { printf("file error\n"); exit(1); }
	// If we hit feof set LastStream to true
	if (feof(FilePtr)) { LastStream=true; }
      }
      // Reset where in stream
      WhereInStream=0;
      
      //printf("Grabbed stream with %d bytes\n", LengthStream);
    }
    // If stream is shorter than Chunk we're definitely in
    // the last stream
    if (LengthStream<FIO_CHUNK) { LastStream=true; }
  }

  //! Get a temporary stream
  unsigned int GetTempStream() {
    long int FL;
    unsigned int TLength;

    FL=ftell(FilePtr);
    TLength=fread(TempStream, sizeof(char), FIO_TEMPCHUNK, FilePtr);
    fseek(FilePtr, FL, SEEK_SET);
    return TLength;
  }
  //! Read a character and move the index along the stream
  char GetChar() {
    char C;
    if (WhereInStream<LengthStream) {
      // In stream - easy
      C=Stream[WhereInStream];
      WhereInStream++;
    } else {
      if (!LastStream) {
	// This isn't the last stream so get new stream and go
	GetStream();
	C=GetChar();
      } else {
	// This is the last stream so set end of file to true
	C=0;
	EndOfFile=true;
      }
    }
    return C;
  }
  //! Read a character but do not move the index along the stream
  char GetTempChar() {
    char C;
    if (WhereInStream<LengthStream) {
      // The easy option
      C=Stream[WhereInStream];
    } else {
      // The hard option - at end of stream
      if (LastStream) {
	// This is the last stream so there is nothing to grab
	// return the last character instead
	C=Stream[LengthStream-1];
      } else {
	if (GetTempStream()>0) {
	  C=TempStream[0];
	}
      }	   
    }
    return C;
  }
};


#endif
