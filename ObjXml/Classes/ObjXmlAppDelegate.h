//
//  ObjXmlAppDelegate.h
//  ObjXml
//
//  Created by John Holdsworth on 08/10/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "RecordView.h"
#import "iTunesItem.h"
#import "objxml.h"

@interface ObjXmlAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    UINavigationController *navigationController;
	IBOutlet UITableView *tableView;
	IBOutlet RecordView *prototype;
	IBOutlet UIView *splash;
	
	int count;
	OOXMLSaxParser parser;
	OOArray<iTunesItem *> rows;
	NSTimeInterval last, fetch, parse, build;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;

- (IBAction)reload;

@end

