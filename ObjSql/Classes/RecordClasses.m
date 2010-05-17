//
//  Records.m
//  DB
//
//  Created by John Holdsworth on 09/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "RecordClasses.h"

@implementation Authors

+ (NSString *)ooTableKey { return @"Au_id"; }
+ (NSString *)ooOrderBy { return @"au_lname, au_fname"; }

// derived field opulated when record loads
- (void)awakeFromDB {
	_au_name = au_fname+" "+au_lname;
}

// derived field calculated "on the fly"
- (NSString *)_zip {
	return &(state+" "+postalcode);
}

- (OOArray<AuthorsTitles *>)titles {
	return [AuthorsTitles selectRecordsRelatedTo:self];
}

- (OOStringArray)blurbs {
	OOArray<Blurbs *> blurbs = [Blurbs selectRecordsRelatedTo:self];
	OOStringArray out;
	for ( int i=0 ; i<blurbs ; i++ )
		out += blurbs[i]->copy_text;
	return out;
}

- (UIImage *)picture {
	OOArray<Au_pix *> pix = [Au_pix selectRecordsRelatedTo:self];
	return pix>0 ? [UIImage imageWithData:pix[0]->pic] : nil;
}

-(void) dealloc {
	[Au_id release];
	[au_lname release];
	[super dealloc];
}


@end

@implementation Au_pix

+ (NSString *)ooTableKey { return @"Au_id"; }

@end

@implementation Blurbs
@end

@implementation Discounts
@end

@implementation Publishers

+ (NSString *)ooTableKey { return @"Pub_id"; }

- (OOArray<Titles *>)titles {
	return [Titles selectRecordsRelatedTo:self];
}

@end

@implementation Roysched
@end

@implementation Sales

+ (NSString *)ooTableKey { return @"Stor_id, Ord_num"; }

- (OOArray<SalesDetail *>)detail {
	return [SalesDetail selectRecordsRelatedTo:self];
}

@end

@implementation SalesDetail

- (OORef<Titles *>)title {
	OOArray<Titles *> titles = [Titles selectRecordsRelatedTo:self];
	return titles[0];
}

- (OORef<Stores *>)store {
	OOArray<Stores *> stores = [Titles selectRecordsRelatedTo:self];
	return stores[0];
}	

@end

@implementation Stores

+ (NSString *)ooTableKey { return @"Stor_id"; }

- (OOArray<Sales *>)orders {
	return [Sales selectRecordsRelatedTo:self];
}

@end

@implementation Titles

+ (NSString *)ooTableKey { return @"Title_id"; }

- (OOArray<TitlesAuthors *>)authors {
	return [TitlesAuthors selectRecordsRelatedTo:self];
}

- (OORef<Publishers *>)publisher {
	OOArray<Publishers *> publishers = [Publishers selectRecordsRelatedTo:self];
	return publishers[0];
}

- (OOArray<SalesDetail *>)sales {
	return [SalesDetail selectRecordsRelatedTo:self];
}

@end

@implementation TitleAuthor
+ (NSString *)ooTableKey { return @"Au_id, Title_id"; }
+ (NSString *)ooConstraints { return @"constraint ta_a foreign key (Au_id) references Authors (Au_id),\n"
							"\tconstraint ta_t foreign key (Title_id) references Titles (Title_id)"; }
@end

@implementation TitlesAuthors 
+ (NSString *) ooTableTitle { return @"Tiles's Authors"; }
+ (NSString *) ooTableSql { return @"create view if not exists TitlesAuthors as "
	"select ta.Title_id, a.* from TitleAuthor as ta, Authors a where ta.Au_id = a.Au_id"; }
@end

@implementation AuthorsTitles 
+ (NSString *) ooTableTitle { return @"Author's Titles"; }
+ (NSString *) ooTableSql { return @"create view if not exists AuthorsTitles as "
	"select ta.Au_id, t.* from TitleAuthor as ta, Titles t where ta.Title_id = t.Title_id"; }
@end

