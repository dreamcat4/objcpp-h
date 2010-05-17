//
//  Catcher.m - print stack trace of exceptions and break to debugger
//  ObjCpp
//
//  Created by John Holdsworth on 27/11/2008.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//
//  $Id: //depot/2.2/ObjCpp/Catcher.m#1 $
//  $DateTime: 2009/11/17 13:00:06 $
//

#ifdef OODEBUG
#import <Foundation/Foundation.h>

@interface NSException(Catcher)
@end

@implementation NSException(Catcher)

// only for use in applications which do not use exceptions routinely themselves...
- (id)initWithName:(NSString *)theName reason:(NSString *)theReason userInfo:(NSDictionary *)userInfo {
	
    @try {
        @throw self;
    }
    @catch ( NSException *ex ) {
        NSMutableString *buffer = [NSMutableString string];
		[buffer appendFormat:@"atos -p %d -printHeader", getpid()];
		
		for( NSNumber *n in [ex callStackReturnAddresses] )
            [buffer appendFormat:@" %u", [n intValue]];
		
		NSLog( @"executing:\n\n%@\n\n", buffer );
		system( [buffer UTF8String] );
    }

    NSLog( @"*** Terminating app due to uncaught exception '%@', reason: '%@'", theName, theReason );

	(*(int *)0)++; // invalid memory access hands control over to debugger...
	return self;
}

@end
#endif
