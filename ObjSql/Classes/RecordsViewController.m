//
//  RecordsViewController.m
//  ObjSql
//
//  Created by John Holdsworth on 10/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "RecordsViewController.h"
#import "JoinsViewController.h"
#import "RecordView.h"

@implementation RecordsViewController

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidAppear:(BOOL)animated {
	self.tableView.editing = NO;
	editButton.title = @"Edit";
	if ( 0 || recordClass )
		rows = [recordClass selectRecordsRelatedTo:parent];
	[self.tableView reloadData];
}

- (void)didSelect:(id)record {
	JoinsViewController *newRootViewController = [[JoinsViewController alloc] initWithNibName:@"JoinsViewController" bundle:nil];

	OOStringArray keys = "recordClass recordEditorController editButton scrollView "
		"Authors Au_pix Blurbs Discounts Publishers Roysched Sales SalesDetail "
		"Stores TitleAuthor Titles TitlesAuthors AuthorsTitles";

	for ( NSString *key in *keys )
		[newRootViewController setValue:[rootViewController valueForKey:key] forKey:key];

	newRootViewController.title = *("Joins from "+[[OODatabase sharedInstance] tableMetaDataForClass:[record class]]->tableTitle);
	newRootViewController->parent = record;

	[self.navigationController pushViewController:newRootViewController animated:YES];
	[newRootViewController release];
}

- (void)viewDidDisappear:(BOOL)animated {	
	if ( [editButton.title isEqualToString:@"Save"] )
		[[[[UIAlertView alloc] initWithTitle:self.title message:@"Discard Edits?" delegate:self
						   cancelButtonTitle:@"Save" otherButtonTitles:@"Discard", nil] autorelease] show];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
	if ( buttonIndex == 0 )
		[OODatabase commit];
	else
		[OODatabase rollback];
}

- (IBAction)saveEdits {
	if ( [editButton.title isEqualToString:@"Edit"] ) {
		editButton.title = @"Cancel";
		self.tableView.editing = YES;
	}
	else {
		if ( [editButton.title isEqualToString:@"Cancel"] ||
			[OODatabase commit] )
			editButton.title = @"Edit";
		self.tableView.editing = NO;
	}
}

#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	self.tableView.rowHeight = 500;
    return rows;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	return prototype.frame.size.height;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {

    NSString *CellIdentifier = NSStringFromClass( [rows[indexPath.row] class] );
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

    if (cell == nil) {
        //cell = [[[RecordCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier prototype:prototype] autorelease];
		cell = [[[UITableViewCell alloc] initWithFrame:CGRectMake(0.0, 0.0,
															 prototype.frame.size.width,
															 prototype.frame.size.height) 
								  reuseIdentifier:CellIdentifier] autorelease];
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		[[cell contentView] addSubview:[prototype copy]];
    }
    
	// Configure the cell.
	RecordView *rv = [[[cell contentView] subviews] objectAtIndex:0];
	rv->highlighted = indexPath.row%2;
	[rv setNeedsDisplay];

	[rows[indexPath.row] bindToView:[cell contentView] delegate:nil];
    return cell;
}

// Override to support row selection in the table view.
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[self didSelect:rows[indexPath.row]];
}

- (void)tableView:(UITableView *)aTableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle 
forRowAtIndexPath:(NSIndexPath *)indexPath {
	[~rows[[indexPath row]] delete];
	editButton.title = @"Save";

	[aTableView beginUpdates];
	[aTableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationTop];
	[aTableView endUpdates];
}

@end
