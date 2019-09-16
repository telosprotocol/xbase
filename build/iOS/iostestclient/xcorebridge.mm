//
//  xcorebridge.cpp
//  iostestclient
//
//  Created by Taylor Wei on 7/12/15.
//  Copyright (c) 2015 Taylor Wei. All rights reserved.
//


#include "xcorebridge.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <string>
#import <UIKit/UIKit.h>

#include "xbase.h"
#include "xsocket.h"

int test_udp();

using namespace top;
using namespace top::base;

bool thread_run(void* args,bool & bStopSignal);


bool quit_process = false;
std::string remote_addr;
std::string  GetDocumentHomeFolder()
{
    NSString *nsDir = [xcorebridge getDocumentDictionary];
    if(nsDir != nil)
    {
        return std::string( [nsDir UTF8String]);
    }
    return std::string();
}


std::string local_date_time()
{
    time_t t = time(NULL);
    struct tm tm = *::localtime(&t);
    
    char szFormatedString[1024];
    snprintf(szFormatedString,1024,"%d:%d:%d",tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    return std::string(szFormatedString);
}

std::string gmt_date_time()
{
    time_t rawtime;
    time(&rawtime); //return how many seconds since 1970/01/01 and 00.00.00
    struct tm tminfo = *::gmtime(&rawtime); //conver to GMT time information(tm)
    
    char szFormatedString[1024];
    snprintf(szFormatedString,1024,"%d:%d:%d",tminfo.tm_hour, tminfo.tm_min, tminfo.tm_sec);
    
    return std::string(szFormatedString);
}

//return seconds from UTC base time
time_t  utility_gmttime()
{
    struct tm* timeinfo = 0;
    time_t rawtime;
    
    time(&rawtime); //return how many seconds since 1970/01/01 and 00.00.00 at local timezone
    timeinfo = ::gmtime(&rawtime); //conver to GMT time information(tm)
    return mktime(timeinfo);//convert again to see how many seconds since 1970/01/01 and 00.00.00 at GMT timezone
}

//return seconds from local base time
time_t utility_localtime()
{
    struct tm* timeinfo = 0;
    time_t rawtime;
    
    time(&rawtime);
    timeinfo = ::localtime(&rawtime);
    return mktime(timeinfo);
}

@implementation xcorebridge
+(NSString*) getDocumentDictionary
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if( (paths != nil) && ([paths count] > 0) )
    {
        NSString * dict = [paths objectAtIndex:0];
        return dict;
    }
    
    return nil;
}

+(void) start
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        //std::string std_log_file = GetDocumentHomeFolder() + "//xcore2-client";
        std::string std_log_file = GetDocumentHomeFolder();
        const char* log_file =  std_log_file.c_str();
 
        xinit_log(log_file, true, true);
        xset_log_level(enum_xlog_level_debug);
        
        bool bStopSignal = false;
        thread_run(NULL,bStopSignal);
    });
}
@end

bool thread_run(void* args,bool & bStopSignal)
{
    int quit_process = 0;
    while(!quit_process)
    {
        test_udp();
        quit_process = 1;
        if(bStopSignal)
        {
            sleep(1);
        }
        else
        {
            sleep(1);
        }
    }
    return 0;
}

