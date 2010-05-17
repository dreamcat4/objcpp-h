//
//  JoinsViewController.m
//  ObjSql
//
//  Created by John Holdsworth on 10/09/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "JoinsViewController.h"

@implementation JoinsViewController

- (BOOL)recordWasEdited {
	return [editButton.title isEqualToString:@"Save"];
}

- (void)viewDidAppear:(BOOL)animated {
	if ( [self recordWasEdited] )
		// there are unsaved edits - Save or Discard Prompt
		[[[[UIAlertView alloc] initWithTitle:recordEditorController.title message:@"Discard Edits?" delegate:self
						   cancelButtonTitle:@"Save" otherButtonTitles:@"Discard", nil] autorelease] show];

	[inspector removeFromSuperview];
	[super viewDidAppear:animated];
	editButton.title = @"Insert";
}

- (void)didSelect:(id)record {
	table = record;

	if ( table->recordClass == [parent class] ) {
		// if user has joined to the same table as the record
		// they selected load the editor view for that class
		inspector = [self valueForKey:table->recordClassName];
		recordEditorController.title = *("Edit of "+table->recordClassName);
		[scrollView addSubview:inspector];
		scrollView.contentSize = inspector.frame.size;
		scrollView.contentOffset = CGPointMake( 0, 0 );
		// load editor elements with data from record
		[parent bindToView:inspector delegate:self];
		if ( self.navigationController.topViewController != recordEditorController )
			[self.navigationController pushViewController:recordEditorController animated:YES];
		editButton.target = self;
		return;
	}
	
	// otherwise display table view of records
	recordListController.title = table->tableTitle;
	recordListController->prototype = [recordListController valueForKey:table->recordClassName];
	recordListController->recordClass = table->recordClass;
	recordListController->parent = parent;

	// clea out previous data
	~recordListController->rows;
	[recordListController.tableView reloadData];

	[self.navigationController pushViewController:recordListController animated:YES];
}

- (void)saveEdits {
	if ( ![self recordWasEdited] ) {
		parent = [table->recordClass insert];
		editButton.title = @"Save";
		[self didSelect:table];
	}		
	// commit any chages to database when user clicks "Save" button
	else if ( [parent commit] == 1 )
		editButton.title = @"Insert";
	else
		[[[[UIAlertView alloc] initWithTitle:@"OODatabase Error" 
									 message:OOFormat( @"%s\n\n%@", 
													  [OODatabase sharedInstance]->errmsg,
													  *[OODatabase sharedInstance]->lastSQL ) 
									delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil] autorelease] show];
}

#pragma mark delegate methods when record data changes on the interface

- (IBAction)valueChanged:(UIView *)view {
	if ( ![self recordWasEdited] ) {
		// user has started editing data - place save button on navigation bar
		editButton.title = @"Save";
		// tell record it is about to be updated and it should take a snapshot
		[parent update];
	}
	[parent updateFromView:view];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[textField resignFirstResponder];
	[self valueChanged:textField];
	return YES;
}

- (void)textFieldDidEndEditing:(UITextField *)textField {
	[self valueChanged:textField];	
}

- (void)textViewDidChange:(UITextView *)textView {
	[self valueChanged:textView];
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
	// list of tables is not editable
	return NO;
}

@end

