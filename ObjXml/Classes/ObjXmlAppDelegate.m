//
//  ObjXmlAppDelegate.m
//  ObjXml
//
//  Created by John Holdsworth on 08/10/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "ObjXmlAppDelegate.h"


@implementation ObjXmlAppDelegate

@synthesize window;
@synthesize navigationController;


#pragma mark -
#pragma mark Application lifecycle

- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    // Override point for customization after app launch    
	[window addSubview: 1 ? [navigationController view] : splash];
    [window makeKeyAndVisible];
	//parser.flags = OOXMLRecursive;
	[self reload];
}

- (NSTimeInterval)elapsed {
	NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
	NSTimeInterval elapsed = now - last;
	last = now;
	return elapsed;
}

- (IBAction)reload {
	~rows;
	[tableView reloadData];
	navigationController.visibleViewController.title = OOFormat( @"Fetching %d Items...", count += 100 );
    // create the connection with the request and start loading the data
	OOURL url = OOFormat(@"http://ax.phobos.apple.com.edgesuite.net/WebObjects/MZStore.woa/wpa/MRSS/newreleases/limit=%d/rss.xml", count);
    [NSURLConnection connectionWithRequest:url.request() delegate:self];
	fetch = parse = build = 0.;
	[self elapsed];
}

// Disable caching so that each time we run this app we are starting with a clean slate.
- (NSCachedURLResponse *)connection:(NSURLConnection *)connection willCacheResponse:(NSCachedURLResponse *)cachedResponse {
    return nil;
}

// Inform user of errors.
- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
	[[[[UIAlertView alloc] initWithTitle:@"Fetch Error"
								 message:[error localizedDescription] delegate:self
					   cancelButtonTitle:@"OK" otherButtonTitles:nil] autorelease] show];
	navigationController.visibleViewController.title = @"Network Required";
	[self performSelector:@selector(load) withObject:nil afterDelay:0.05];
}

// use saved results when network not available.
- (void)load {
	rows = [OODatabase select:nil intoClass:[iTunesItem class] joinFrom:nil];
	[tableView reloadData];	
}

// Called when a chunk of data has been downloaded to parse XML inrementally
- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    // Parse the downloaded chunk of data.
	fetch += [self elapsed];
    parser.parse( data );
	parse += [self elapsed];
}

// Download is complete
- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
	navigationController.visibleViewController.title = @"Building instances...";
	[self performSelector:@selector(build) withObject:nil afterDelay:0.05];
}

// select out items from xml body
- (void)build {
	fetch += [self elapsed];
	OONode doc = parser.rootNodeForXMLData();
	navigationController.visibleViewController.title = **doc["rss/channel/title"];
	const char *xpath = parser.flags & OOXMLRecursive ? "//item" : "rss/channel/item";
	OOStringDictionaryArray items = doc[xpath].dictionaries();
	rows = [OOMetaData import:items intoClass:[iTunesItem class]];
	build += [self elapsed];

	NSLog( @"Fetch: %f, parse: %f, build: %f", fetch, parse, build );
	[tableView reloadData];
	if ( 0 )
		[self reload];
	else
		[self performSelectorInBackground:@selector(save) withObject:nil];
}

// save fetched items to sqlite database for use off-line
- (void)save {
	static BOOL alreadyRunning;
	if ( alreadyRunning )
		return;
	alreadyRunning = YES;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSTimeInterval start = [NSDate timeIntervalSinceReferenceDate];
	[OODatabase exec:"delete from iTunesItem"];
	[OODatabase insertArray:rows];
	[OODatabase commit];
	NSLog( @"Save: %f", [NSDate timeIntervalSinceReferenceDate]-start );
	alreadyRunning = NO;
	[pool release];
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

#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)aTableView numberOfRowsInSection:(NSInteger)section {
	tableView.rowHeight = 500;
    return rows;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	return prototype.frame.size.height;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)aTableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
    NSString *CellIdentifier = NSStringFromClass( [rows[indexPath.row] class] );
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
	
    if (cell == nil) {
        //cell = [[[RecordCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier prototype:prototype] autorelease];
		cell = [[[UITableViewCell alloc] initWithFrame:CGRectMake(0.0, 0.0,
																  prototype.frame.size.width,
																  prototype.frame.size.height) 
									   reuseIdentifier:CellIdentifier] autorelease];
		[[cell contentView] addSubview:[prototype copy]];
    }
    
	// Configure the cell.
	RecordView *rv = [[[cell contentView] subviews] objectAtIndex:0];
	rv->highlighted = indexPath.row%2;
	[rv setNeedsDisplay];
	
	[OOMetaData bindRecord:rows[indexPath.row] toView:[cell contentView] delegate:nil];
    return cell;
}

- (NSIndexPath *)tableView:(UITableView *)aTableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	return nil;
}

@end

