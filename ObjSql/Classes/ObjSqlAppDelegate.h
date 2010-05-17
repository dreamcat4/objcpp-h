//
//  ObjSqlAppDelegate.h
//  ObjSql
//
//  Created by John Holdsworth on 10/09/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "JoinsViewController.h"

@interface ObjSqlAppDelegate : NSObject <UIApplicationDelegate> {
    
    UIWindow *window;
    UINavigationController *navigationController;
	IBOutlet JoinsViewController *rootViewController;
	IBOutlet UIViewController *infoController;
	IBOutlet UIWebView *webView;
	IBOutlet UIView *splash;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;

- (IBAction)info;

@end

