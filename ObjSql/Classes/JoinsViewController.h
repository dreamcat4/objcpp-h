//
//  JoinsViewController.h
//  ObjSql
//
//  Created by John Holdsworth on 10/09/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "RecordsViewController.h"

@interface JoinsViewController : RecordsViewController<UITextFieldDelegate,UITextViewDelegate> {
	IBOutlet RecordsViewController *recordListController;
	IBOutlet UIViewController *recordEditorController;
	IBOutlet UIScrollView *scrollView;
	UIView *inspector;
	OOMetaData *table;
}

- (IBAction)valueChanged:(UIView *)view;

@end
