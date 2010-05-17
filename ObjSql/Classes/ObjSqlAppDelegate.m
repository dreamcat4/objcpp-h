//
//  ObjSqlAppDelegate.m
//  ObjSql
//
//  Created by John Holdsworth on 10/09/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "ObjSqlAppDelegate.h"
#import "JoinsViewController.h"


@implementation ObjSqlAppDelegate

@synthesize window;
@synthesize navigationController;


#pragma mark -
#pragma mark Application lifecycle

- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    
    // Override point for customization after app launch    
	webView.backgroundColor = [UIColor clearColor];
	[window addSubview:1?[navigationController view]:splash];
    [window makeKeyAndVisible];

	[self performSelector:@selector(loadRoot) withObject:nil afterDelay:0];
}

- (void)loadRoot {
	OOStringArray recordClassNames = [[OODatabase sharedInstance] registerSubclassesOf:[OORecord class]];

	if ( [[OODatabase sharedInstance] stringForSql:"select count(*) from Authors"] == "0" ) {
		for ( NSString *table in *recordClassNames ) {
			OOResource flatFile( OOFormat("%@.txt", [table lowercaseString]) );
			if ( !flatFile.exists() )
				continue;
			[OODatabase exec:"delete from %@",table];
			Class recordClass = [[NSBundle mainBundle] classNamed:table];
			int imported = [recordClass importFrom:flatFile delimiter:"~"],
			exported = [OOMetaData export:[recordClass select] delimiter:"~"] / "\n";
			NSLog( @"Imported: %@, records: %d c.f. %d", table, imported, exported-1 );
		}
	}

	rootViewController.title = @"Example Tables";
	rootViewController->recordClass = [OOMetaData class];
	[rootViewController viewDidAppear:YES];
}

- (IBAction)info {
	[rootViewController.navigationController pushViewController:infoController animated:YES];
	[webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://objcpp.johnholdsworth.com/info.html"]]];
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
	if ( [[[request URL] absoluteString] rangeOfString:@"mailto:"].location == 0 ) {
		[[UIApplication sharedApplication] openURL:[request URL]];
		return NO;
	}
	return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Save data if appropriate
}


#pragma mark -
#pragma mark Memory management

- (void)dealloc {
	[navigationController release];
	[window release];
	[super dealloc];
}


@end

