//
//  RecordsViewController.h
//  ObjSql
//
//  Created by John Holdsworth on 10/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "RecordClasses.h"

@class JoinsViewController;

@interface RecordsViewController : UITableViewController<UITableViewDelegate,UITableViewDataSource> {
	IBOutlet JoinsViewController *rootViewController;
	IBOutlet UIBarButtonItem *editButton;
	IBOutlet UIView *prototype;

	OOArray<OORecord *> rows;
	
	// outlets to record cell prototypes (and editors)
	IBOutlet UIView *Authors, *Au_pix, *Blurbs, *Discounts, *Publishers, 
	*Roysched, *Sales, *SalesDetail, *Stores, *TitleAuthor, *Titles, 
	*TitlesAuthors, *AuthorsTitles;

@public
	Class recordClass;
	OOReference<OORecord *> parent;
}

- (IBAction)saveEdits;

@end
