#pragma warning(disable : 4996)
#pragma warning(disable : 4244) //type conversion

#include "error.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

#define ERR_STREAM stdout

#ifdef SKIP_PARAMS_FILE
#define SHOW_PROGRESS_MESSAGES_ 0
#define SHOW_DEBUG_MESSAGES_    0
#else
int _g_params_inited = 0;

int check_params_loaded()
{ char f[] = "default.parameters";
  if(_g_params_inited==0)
  { if(Load_Params_File(f))
    { 
      warning("Make sure %s is in the calling directory\n"  
              "Could not load parameters from file: %s\n"
              "Writing defaults to current directory.\n"
              "\tTrying again...\n",f,f);
      Print_Params_File(f);
      if(Load_Params_File(f))
      { warning("\tStill couldn't load parameters from file.\n");
        return 0;
      }
      _g_params_inited = 1;
      return 1;
    }
    else
      _g_params_inited = 1;
  }
  return 1;
}
#endif

SHARED_EXPORT
void error(char *str, ... )
{
  va_list argList;
  va_start( argList, str );
  fprintf(ERR_STREAM, "*** ERROR: ");
  vfprintf(ERR_STREAM, str, argList);
  va_end( argList );
  fflush(NULL);
  exit(-1);
}

SHARED_EXPORT
void warning(char *str, ... )
{
  va_list argList;
  va_start( argList, str );
  fprintf(ERR_STREAM, "--- Warning: ");
  vfprintf(ERR_STREAM, str, argList);
  va_end( argList );
  fflush(NULL);
}

SHARED_EXPORT
void debug(char *str, ... )
{
  va_list argList;
  va_start( argList, str );
#ifdef SKIP_PARAMS_FILE
  if(SHOW_DEBUG_MESSAGES_)
#else
  if(check_params_loaded() && SHOW_DEBUG_MESSAGES)
#endif
    vfprintf(ERR_STREAM, str, argList);
  va_end( argList );
  fflush(NULL);
}

SHARED_EXPORT
void help(int show, char *str, ... )
{ if(show)
  { va_list argList;
    Print_Argument_Usage(ERR_STREAM,0);
    va_start( argList, str );
    vfprintf(ERR_STREAM, str, argList);
    va_end( argList );
    fflush(NULL);
	exit(0);
  }
}

SHARED_EXPORT
void progress(char *str, ... )
{ va_list argList;
  va_start( argList, str );
#ifdef SKIP_PARAMS_FILE
  if(SHOW_PROGRESS_MESSAGES_)
#else
  if(check_params_loaded() && SHOW_PROGRESS_MESSAGES )
#endif
    vfprintf( ERR_STREAM, str, argList);
  va_end( argList );
  fflush(NULL);
}

SHARED_EXPORT
void progress_meter(double cur, double min, double max, int len, char *str, ...)
{ 
#ifdef SKIP_PARAMS_FILE
  if(SHOW_PROGRESS_MESSAGES_)
#else
  if(check_params_loaded() && SHOW_PROGRESS_MESSAGES )
#endif
  { va_list argList;
    char buf[1024];
    int n=0;

    {
      va_start( argList, str );
      n = sprintf(buf,"\r");
      n += vsprintf(buf+n, str, argList);
      va_end( argList );
    }


    n += sprintf(buf+n,"[");
    len-=(n-1);
    { 
      int nc = (len)*(cur-min)/(max-min);
      len -= (nc+1);
      while(nc--  > 0) 
        n+=sprintf(buf+n,"|");
      while(len-- > 0)
        n+=sprintf(buf+n,"-");
    }
    n+=sprintf(buf+n,"]\r");
    buf[n] = '\0';

    fprintf(ERR_STREAM,buf);
    fflush(NULL);
  }
}
