//
//  ObjCppAppDelegate.h
//  ObjCpp
//
//  Created by John Holdsworth on 14/04/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "objvec.h"
#import "objsql.h"
#import "objxml.h"

@interface OOTestObjC : NSObject {
	
}
@end

@interface ObjCppAppDelegate : NSObject {
	IBOutlet NSTextField *results;
	OOReference<OOTestObjC *> ivarRef;
	OOArray<OOTestObjC *> ivarArray;
	OODictionary<OOTestObjC *> ivarDict;
	OOString ivarString;
	OOVector<double> ivarVector;
	OOClassDict<OOClassTest> ivarClassDict;
}

@end
