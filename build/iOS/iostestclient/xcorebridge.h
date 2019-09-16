//
//  xcorebridge.h
//  iostestclient
//
//  Created by Taylor Wei on 7/12/15.
//  Copyright (c) 2015 Taylor Wei. All rights reserved.
//

#ifndef __iostestclient__xcorebridge__
#define __iostestclient__xcorebridge__

#include <stdio.h>
#import <Foundation/Foundation.h>


@interface xcorebridge : NSObject
{
}
+(void)         start;
+(void)         start1;
+(NSString*)    getDocumentDictionary;
@end
 

#endif /* defined(__iostestclient__xcorebridge__) */
