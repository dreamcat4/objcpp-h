/*
 *  objsql.m - implementaion simple persistence layer using objcpp.h
 *  ========
 *
 *  Created by John Holdsworth on 01/04/2009.
 *  Copyright 2009 John Holdsworth.
 *
 *  $Id: //depot/2.2/ObjCpp/objsql.mm#1 $
 *  $DateTime: 2009/11/17 13:00:06 $
 *
 *  C++ classes to wrap up XCode classes for operator overload of
 *  useful operations such as access to NSArrays and NSDictionary
 *  by subscript or NSString operators such as + for concatenation.
 *
 *  This works as the Apple Objective-C compiler supports source
 *  which mixes C++ with objective C. To enable this: for each
 *  source file which will include/import this header file, select
 *  it in Xcode and open it's "Info". To enable mixed compilation,
 *  for the file's "File Type" select: "sourcecode.cpp.objcpp".
 *
 *  For bugs or ommisions please email objcpp@johnholdsworth.com
 *
 *  Home page for updates and docs: http://objcpp.johnholdsworth.com
 *
 *  Forum for support: http://n2.nabble.com/Objcpp-Q-A-f2626793.html 
 *
 *  You may make commercial use of this source in applications without
 *  charge but not sell it as source nor can you remove this notice from
 *  this source if you redistribute. You can make any changes you like
 *  to this code before redistribution but please annotate them below.
 *
 *  If you find it useful please send a donation via paypal to account
 *  objcpp@johnholdsworth.com. Thanks.
 *
 *  THIS CODE IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND EITHER
 *  EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 *  WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS
 *  THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING
 *  ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT
 *  OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 *  TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED 
 *  BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH 
 *  ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 */

#import <objc/runtime.h>
#import <sqlite3.h>

#import "objsql.h"

#if 0
#ifdef OODEBUG
#define OODEBUG_SQL  1
#endif
#endif

OOOODatabase OODB;

#pragma mark OORecord abstract superclass for records

@implementation OORecord

+ (id)insert { 
	OORecord *record = [[self alloc] init];
	[record insert];
	[record release];
	return record;	
}

+ (id)insertWithParent:(id)parent {
	return [[OODatabase sharedInstance] copyJoinKeysFrom:parent to:[self insert]];
}

- (id)insert { [[OODatabase sharedInstance] insert:self]; return self; }
- (id)delete { [[OODatabase sharedInstance] delete:self]; return self; }

- (void)update { [[OODatabase sharedInstance] update:self]; }
- (void)indate { [[OODatabase sharedInstance] indate:self]; }
- (void)upsert { [[OODatabase sharedInstance] upsert:self]; }

- (int)commit { return [[OODatabase sharedInstance] commit]; }
- (int)rollback { return [[OODatabase sharedInstance] rollback]; }

// to handle null values for ints, floats etc.
- (void)setNilValueForKey:(NSString *)key {
	static OOReference<NSValue *> zeroForNull;
	if ( !zeroForNull )
		zeroForNull = [NSNumber numberWithInt:0];
	[self setValue:zeroForNull forKey:key];
}

+ (OOArray<id>)select {
	return [[OODatabase sharedInstance] select:nil intoClass:self joinFrom:nil];
}

+ (OOArray<id>)select:(const OOString &)sql {
	return [[OODatabase sharedInstance] select:sql intoClass:self joinFrom:nil];
}

+ (OOArray<id>)selectRecordsRelatedTo:(id)parent {
	return [[OODatabase sharedInstance] select:nil intoClass:self joinFrom:parent];
}

+ (id)record {
	return [[[self alloc] init] autorelease];
}

- (OOArray<id>)select {
	return [[OODatabase sharedInstance] select:nil intoClass:[self class] joinFrom:self];
}

/**
 import a flat file with column values separated by the delimiter specified into 
 the table associated with this class.
 */

+ (int)importFrom:(const OOFile &)file delimiter:(const OOString &)delim {
	OOArray<id> rows = [OOMetaData import:file.string() intoClass:self delimiter:delim];
	[[OODatabase sharedInstance] insertArray:rows];
	return [OODatabase commit];
}

/**
 Export a flat file with all rows in the table associated with the record subclass.
 */

+ (BOOL)exportTo:(const OOFile &)file delimiter:(const OOString &)delim {
	return file.save( [OOMetaData export:[self select] delimiter:delim] );
}

/**
 populate a view with the string values taken from the ivars of the record
 */

- (void)bindToView:(UIView *)view delegate:(id)delegate {
	[OOMetaData bindRecord:self toView:view delegate:delegate];
}

/**
 When the delegate method os sent use this method to update the record's values
 */

- (void)updateFromView:(UIView *)view {
	[OOMetaData updateRecord:self fromView:view];
}

/**
 Default description is dictionary containing values of all ivars
 */

- (NSString *)description {
	OOMetaData *metaData = [OOMetaData metaDataForClass:[self class]];
	// hack required where record contains a field "description" to avoid recursion
	OOStringArray ivars; ivars <<= *metaData->ivars; ivars -= "description";
	return [*[metaData encode:[self dictionaryWithValuesForKeys:ivars]] description];
}

@end

#pragma mark OOAdaptor - all methods required by objsql to access a database

/**
 An internal class representing the interface to a particular database, in this case sqlite3.
 */

@interface OOAdaptor : NSObject {
	OODatabase *owner;

	sqlite3 *db;
	sqlite3_stmt *stmt;
	struct _str_link { 
		struct _str_link *next; char str[1]; 
	} *strs;
}

- (OOAdaptor *)initPath:(const OOString &)path database:(OODatabase *)database;
- (BOOL)prepare:(const OOString &)sql;

- (BOOL)bindCols:(const OOStringArray &)columns values:(const OODictionary<NSValue *> &)values startingAt:(int)pno bindNulls:(BOOL)bindNulls;
- (OOArray<id>)bindResultsIntoInstancesOfClass:(Class)recordClass metaData:(OOMetaData *)metaData;
- (sqlite_int64)lastInsertRowID;

@end

@interface NSData(OOExtras)
- initWithDescription:(NSString *)description;
@end

#pragma mark OODatabase is the low level interface to a particular database

@implementation OODatabase

static OOReference<OODatabase *> sharedInstance;

/**
 By default database file is "objsql.db" in the user/application's "Documents" directory
 and a single shared OODatabase instance used for all db operations.
 */

+ (OODatabase *)sharedInstance {
	return !sharedInstance ? 
	[self sharedInstanceForPath:OODocument("objsql.db").path()] : *sharedInstance;
}

/**
 Shared instance can be switched between any file paths.
 */
 
 + (OODatabase *)sharedInstanceForPath:(const OOString &)path {
	 [sharedInstance = !path ? (id)kCFNull : [[OODatabase alloc] initPath:path] release];
	 return sharedInstance;
}

+ (BOOL)exec:(const OOString &)fmt, ... {
	va_list argp; va_start(argp, fmt);
	NSString *sql = [[[NSString alloc] initWithFormat:fmt arguments:argp] autorelease];
	va_end( argp );
	return [[self sharedInstance] exec:sql];
}

+ (OOArray<id>)select:(const OOString &)select intoClass:(Class)recordClass joinFrom:(id)parent {
	return [[self sharedInstance] select:select intoClass:recordClass joinFrom:parent];
}
+ (OOArray<id>)select:(const OOString &)select intoClass:(Class)recordClass {
	return [[self sharedInstance] select:select intoClass:recordClass joinFrom:nil];
}
+ (OOArray<id>)select:(const OOString &)select {
	return [[self sharedInstance] select:select intoClass:nil joinFrom:nil];
}

+ (int)insertArray:(const OOArray<id> &)objects { return [[self sharedInstance] insertArray:objects]; }
+ (int)deleteArray:(const OOArray<id> &)objects { return [[self sharedInstance] deleteArray:objects]; }

+ (int)insert:(id)object { return [[self sharedInstance] insert:object]; }
+ (int)delete:(id)object { return [[self sharedInstance] delete:object]; }
+ (int)update:(id)object { return [[self sharedInstance] update:object]; }

+ (int)indate:(id)object { return [[self sharedInstance] indate:object]; }
+ (int)upsert:(id)object { return [[self sharedInstance] upsert:object]; }

+ (int)commit { return [[self sharedInstance] commit]; }
+ (int)rollback { return [[self sharedInstance] rollback]; }
+ (int)commitTransaction { return [[OODatabase sharedInstance] commitTransaction]; }

/**
 Designated initialiser for OODatabase instances. Generally only the shared instance is used
 and the OODatabase class object is messaged instead.
 */

- (OODatabase *)initPath:(const OOString &)path {
	if ( self = [super init] ) {
		[adaptor = [[OOAdaptor alloc] initPath:path database:self] release];
	}
	return self;
}

/**
 Automatically register all classes which are subclasses of a record abstract superclass (e.g. OORecord).
 */

- (OOStringArray)registerSubclassesOf:(Class)recordSuperClass {
	int numClasses = objc_getClassList( NULL, 0 );
	Class *classes = (Class *)malloc( sizeof *classes * numClasses );
	OOArray<Class> viewClasses;
	OOStringArray classNames;
	
	// scan all registered classes for relevant subclasses
	numClasses = objc_getClassList( classes, numClasses );
	for ( int c=0 ; c<numClasses ; c++ ) {
		Class superClass = classes[c];
		while ( superClass = class_getSuperclass( superClass ) )
			if ( superClass == recordSuperClass ) {
				if ( [classes[c] respondsToSelector:@selector(ooTableSql)] )
					viewClasses += classes[c];
				else {
					[[OODatabase sharedInstance] tableMetaDataForClass:classes[c]];
					classNames += class_getName( classes[c] );
				}
				break;
			}
	}
	
	// delay creation views until after tables
	for ( int c=0 ; c<viewClasses ; c++ ) {
		[[OODatabase sharedInstance] tableMetaDataForClass:viewClasses[c]];
		classNames += class_getName( viewClasses[c] );
	}
	
	// return classes in order registered
	free( classes );
	return classNames;
}

/**
 Register a list of classes before using them so OODatabase can determine the relationships between them.
 */

- (void)registerTableClassesNamed:(const OOStringArray &)classes {
	for ( NSString *tableClass in *classes )
		[self tableMetaDataForClass:[[NSBundle mainBundle] classNamed:tableClass]];
}

/**
 Send any SQL to the database. Sql is a format string so escape any '%' characters using '%%'.
 Any results returned are placed as an array of dictionary values in the database->results.
 */
 
- (BOOL)exec:(const OOString &)fmt, ... {
	va_list argp; va_start(argp, fmt);
	NSString *sql = [[NSString alloc] initWithFormat:fmt arguments:argp];
	va_end( argp );
	results = [self select:sql intoClass:NULL joinFrom:nil];
	[sql release];
	return !errcode;
}

/**
 Return a single value from row 1, column one from sql sent to the database as a string.
 */

- (OOString)stringForSql:(const OOString &)fmt, ... {
	va_list argp; va_start(argp, fmt);
	NSString *sql = [[[NSString alloc] initWithFormat:fmt arguments:argp] autorelease];
	va_end( argp );
	if( [self exec:"%@", sql] && results > 0 ) {
		NSString *aColumnName = [[**results[0] allKeys] objectAtIndex:0];
		return [(*results[0])[aColumnName] stringValue];
	}
	else
		return nil;
}

/**
 Used to initialise new child records automatically from parent in relation.
 */

- (id)copyJoinKeysFrom:(id)parent to:(id)newChild {
	OOMetaData *parentMetaData = [self tableMetaDataForClass:[parent class]],
			*childMetaData = [self tableMetaDataForClass:[newChild class]];
	OOStringArray commonColumns = [parentMetaData naturalJoinTo:childMetaData->columns]; ////
	OODictionary<NSValue *> keyValues = [parentMetaData encode:[parent dictionaryWithValuesForKeys:commonColumns]];
	[newChild setValuesForKeysWithDictionary:[childMetaData decode:keyValues]];
	return newChild;
}

/**
 Build a where clause for the columns specified.
 */

- (OOString)whereClauseFor:(const OOStringArray &)columns values:(const OODictionary<NSValue *> &)values qualifyNulls:(BOOL)qualifyNulls {
	OOString out;
	for ( int i=0 ; i<columns ; i++ ) {
		NSString *name = *columns[i];
		const char *prefix = i==0 ?"\nwhere":" and";
		
		id value = values[name];
		if ( value == (id)kCFNull )
			out += qualifyNulls ? OOFormat( @"%s\n\t%@ is NULL", prefix, name ) : @"";
		else if ( !qualifyNulls && [value isKindOfClass:[NSString class]] && [value rangeOfString:@"%"].location != NSNotFound )
			out += OOFormat( @"%s\n\t%@ LIKE ?", prefix, name );
		else
			out += OOFormat( @"%s\n\t%@ = ?", prefix, name );
	}
	return out;
}

/**
 Prepare the sql passed in adding a where clause with bindings for a join to values taken from the parent record.
 */

- (BOOL)prepareSql:(OOString &)sql joinFrom:(id)parent toTable:(OOMetaData *)metaData {
	OODictionary<NSValue *> joinValues;
	OOStringArray sharedColumns;

	if ( parent ) {
		OOMetaData *parentMetaData = [self tableMetaDataForClass:[parent class]];
		sharedColumns = [parentMetaData naturalJoinTo:metaData->joinableColumns];
		joinValues = [parentMetaData encode:[parent dictionaryWithValuesForKeys:sharedColumns]];
	
		sql += [self whereClauseFor:sharedColumns values:joinValues qualifyNulls:NO];
	}

	if ( [metaData->recordClass respondsToSelector:@selector(ooOrderBy)] )
		sql += OOFormat( @"\norder by %@", [metaData->recordClass ooOrderBy] );

#ifdef OODEBUG_SQL
	NSLog( @"prepareSql: %@\n%@", *sql, *joinValues );
#endif

	if ( ![*adaptor prepare:sql] )
		return NO;

	return !parent || [*adaptor bindCols:sharedColumns values:joinValues startingAt:1 bindNulls:NO];
}

/**
 Determine a list of the tables which have a natural join to the record passed in.
 If the record is a specific instance of from a table this should determine if
 there are any record which exist using the join.
 */

- (OOArray<OOMetaData *>)tablesRelatedByNaturalJoinFrom:(id)record {
	OOMetaData *metaData = [record class] == [OOMetaData class] ? 
	record : [self tableMetaDataForClass:[record class]];

	OOStringArray tablesWithNaturalJoin;
	tablesWithNaturalJoin <<= metaData->tablesWithNaturalJoin;

	if ( record && record != metaData )
		for ( int i=0 ; i<tablesWithNaturalJoin ; i++ ) {
			OOString sql = OOFormat( "select count(*) as result from %@", **tablesWithNaturalJoin[i] );
			OOMetaData *childMetaData = tableMetaDataByClassName[tablesWithNaturalJoin[i]];

			[self prepareSql:sql joinFrom:record toTable:childMetaData];
			OOArray<OODictionary<NSValue *> > tmpResults = [*adaptor bindResultsIntoInstancesOfClass:NULL metaData:nil];

			if ( ![(*tmpResults[0])["result"] intValue] )
				~tablesWithNaturalJoin[i--];
		}

	return tableMetaDataByClassName[(OOSlice)+tablesWithNaturalJoin];
}

/**
 Perform a select from a table on the database using either the sql specified 
 orselect all columns from the table associated with the record class passed in.
 If a parent is passed in make a natural join from that record.
 */

- (OOArray<id>)select:(const OOString &)select intoClass:(Class)recordClass joinFrom:(id)parent {
	OOMetaData *metaData = [self tableMetaDataForClass:recordClass ? recordClass : [parent class]];
	OOString sql = !select ? OOFormat( @"select %@\nfrom %@", *(metaData->outcols/", "), *metaData->tableName ) : *select;

	if ( ![self prepareSql:sql joinFrom:parent toTable:metaData] )
		return nil;

	return [*adaptor bindResultsIntoInstancesOfClass:recordClass metaData:metaData];
}

- (OOArray<id>)select:(const OOString &)select intoClass:(Class)recordClass {
	return [self select:select intoClass:recordClass joinFrom:nil];
}

- (OOArray<id>)select:(const OOString &)select {
	return [self select:select intoClass:nil joinFrom:nil];
}

/**
 Returns sqlite3 row identifier for a record instance.
 */

- (long long)rowIDForRecord:(id)record {
	OOMetaData *metaData = [self tableMetaDataForClass:[record class]];
	OOString sql = OOFormat( "select ROWID from %@", *metaData->tableName );
	OOArray<OODictionary<NSNumber *> > idResults = [self select:sql intoClass:nil joinFrom:record];
	return [*(*idResults[0])[@"rowid"] longLongValue];
}

/**
 Returns sqlite3 row identifier for last inserted record.
 */

- (long long)lastInsertRowID {
	return [*adaptor lastInsertRowID];
}

/**
 Insert an array of record objects into the database. This needs to be commited to take effect.
 */

- (int)insertArray:(const OOArray<id> &)objects {
	int count = 0;
	for ( id object in *objects )
		count = [self insert:object];
	return count;
}

/**
 Delete an array of record objects from the database. This needs to be commited to take effect.
 */

- (int)deleteArray:(const OOArray<id> &)objects {
	int count = 0;
	for ( id object in *objects )
		if ( ![object respondsToSelector:@selector(delete)] )
			count = [self delete:object];
		else {
			[object delete];
			count++;
		}
	return count;
}

/**
 Insert the values of the record class instance at the time this method was called into the db.
 (must be commited to take effect). Returns the total number of outstanding inserts/updates/deletes.
 */

- (int)insert:(id)record {
	return transaction += OODictionary<NSValue *>( @"__OOOBJECT__", record, @"__ISINSERT__", kCFNull, nil );
}

/**
 Use the values of the record instance at the time this method is called in a where clause to 
 delete from the database when commit is called. Returns the total number of outstanding
 inserts/updates/deletes.
 */

- (int)delete:(id)record {
	return transaction += OODictionary<NSValue *>( @"__OOOBJECT__", record, nil );
}

/**
 Call this method if you intend to make changes to the record object and save them to the database.
 This takes a snapshot of the previous values to use as a key for the update operation when "commit"
 is called. Returns the total number of outstanding inserts/updates/deletes.
 */

- (int)update:(id)record {
	OOMetaData *metaData = [self tableMetaDataForClass:[record class]];
	OODictionary<NSValue *> oldValues = [metaData encode:[record dictionaryWithValuesForKeys:metaData->columns]];
	for ( NSString *key in *metaData->tocopy )
		[oldValues[key] = [oldValues[key] copy] release];
	oldValues[@"__ISUPDATE__"] = (id)kCFNull;
	oldValues[@"__OOOBJECT__"] = record;
	return transaction += oldValues;
}

/**
 Inserts a record into the database the deletes any previous record with the same key.
 This ensures the record's rowid changes if this is used by child records.
 */

- (int)indate:(id)record {
	OOMetaData *metaData = [self tableMetaDataForClass:[record class]];
	OOString sql = OOFormat( "select rowid from %@", *metaData->tableName );
	OOArray<id> existing = [self select:sql intoClass:nil joinFrom:record];
	int count = [self insert:record];
	for ( NSDictionary *exist in *existing ) {
		OOString sql = OOFormat( @"delete from %@ where rowid = %ld", *metaData->tableName, 
								[[exist objectForKey:@"rowid"] longLongValue] );
		transaction += OODictionary<NSValue *>( @"__OOEXEC__", *sql, nil );
	}
	return count;
}

/**
 Inserts a record into the database unless another record with the same key column values
 exists in which case it will do an update of the previous record (preserving the ROWID.)
 */

- (int)upsert:(id)record {
	OOArray<id> existing = [self select:nil intoClass:[record class] joinFrom:record];
	if ( existing > 1 )
		OOWarn( @"upsert: Duplicate record for upsert: @%", record );
	if ( existing > 0 ) {
		[self update:existing[0]];
		(*transaction[-1])[@"__OOOBJECT__"] = record;
		return transaction;
	}
	else
		return [self insert:record];
}

/**
 Commit all pending inserts, updates and deletes to the database. Use commitTransactin to perform 
 this inside a database transaction.
 */

- (int)commit {
	int commited = 0;

	for ( int i=0 ; i<transaction ; i++ ) {
		OODictionary<NSValue *> values = transaction[i];
		OOString exec = (NSMutableString *)~values[@"__OOEXEC__"];
		if ( !!exec ) {
			if ( ![self exec:@"%@", *exec] )
				OOWarn( @"commit: Error in transaction exec: %@ - %s", *exec, errmsg );
			continue;
		}

		OORef<NSObject *> object = *values[@"__OOOBJECT__"]; values -= @"__OOOBJECT__";
		BOOL isInsert = !!~values[@"__ISINSERT__"], isUpdate = !!~values[@"__ISUPDATE__"];

		OOMetaData *metaData = [self tableMetaDataForClass:[*object class]];
		OODictionary<NSValue *> newValues = [metaData encode:[*object dictionaryWithValuesForKeys:metaData->columns]];
		OOStringArray changedCols;

		if ( isUpdate ) {
			for ( NSString *name in *metaData->columns )
				if ( ![*newValues[name] isEqual:values[name]] )
					changedCols += name;
		}
		else 
			values = newValues;

		OOString sql = isInsert ? 
		OOFormat( @"insert into %@ (%@) values (", *metaData->tableName, *(metaData->columns/", ") ) :
		OOFormat( isUpdate ? @"update %@ set" : @"delete from %@", *metaData->tableName );

		int nchanged = changedCols;
		if ( isUpdate && nchanged == 0 ) {
			OOWarn( @"%s %@", errmsg = (char *)"commit: Update of unchanged record", *object, *(lastSQL = sql) );
			continue;
		}
		
		for ( int i=0 ; i<nchanged ; i++ )
			sql += OOFormat( @"%s\n\t%@ = ?", i==0 ? "" : ",", **changedCols[i] );

		if ( isInsert ) {
			OOString quote = "?", commaQuote = ", ?";
			for ( int i=0 ; i<metaData->columns ; i++ )
				sql += i==0 ? quote : commaQuote;
			sql += ")";
		}
		else
			sql += [self whereClauseFor:metaData->columns values:values qualifyNulls:YES];

#ifdef OODEBUG_SQL
		NSLog( @"commiting: %@ %@", *sql, *values );
#endif

		if ( ![*adaptor prepare:sql] )
			continue;

		if ( isUpdate )
			[*adaptor bindCols:changedCols values:newValues startingAt:1 bindNulls:YES];
		[*adaptor bindCols:metaData->columns values:values startingAt:1+nchanged bindNulls:isInsert];

		[*adaptor bindResultsIntoInstancesOfClass:nil metaData:metaData];
		commited += updateCount;
	}

	transaction = nil;
	return commited;
}

/**
 Commit all pending inserts, updates, deletes to the database inside a transaction.
 */

- (int)commitTransaction {
	[self exec:"BEGIN TRANSACTION"];
	int updated = [self commit];
	return [self exec:"COMMIT"] ? updated : 0;
}

/**
 Rollback any outstanding inserts, updates, or deletes. Please note updated values 
 are also rolled back inside the actual record in the application as well.
 */

- (int)rollback {
	for ( NSMutableDictionary *d in *transaction ) {
		OODictionary<id> values = d;

		if ( !!~values[@"__ISUPDATE__"] ) {
			OORef<OORecord *> record = ~values[@"__OOOBJECT__"];
			OOMetaData *metaData = [self tableMetaDataForClass:[*record class]];
		
			for ( NSString *name in *metaData->boxed )
				[(id)[[*record valueForKey:name] pointerValue] release];

			[*record setValuesForKeysWithDictionary:[metaData decode:values]];
		}
	}
	return [*~transaction count];
}

/**
 Find/create an instance of the OOMetaData class which describes a record class and its associated table.
 If the table does not exist it will be created along with indexes for columns/ivars which have 
 upper case names for use in joins. Details of the tables parameters can be controlled by using
 methods in the OOTableCustomisation protool. If it does not exist a meta table class which
 prepresents OOMetaData records themselves is also created from which the list of all registered
 tables can be selected.
 */
 
- (OOMetaData *)tableMetaDataForClass:(Class)recordClass {
	if ( !recordClass || recordClass == [OOMetaData class] )
		return [OOMetaData metaDataForClass:[OOMetaData class]];

	NSString *className = NSStringFromClass( recordClass );
	OOMetaData *metaData = tableMetaDataByClassName[className];
	
	if ( !metaData ) {
		metaData = [OOMetaData metaDataForClass:recordClass];
		
#ifdef OODEBUG_SQL
		NSLog(@"\n%@", *metaData->createTableSQL);
#endif
		
		if ( metaData->tableName[0] != '_' && 
			[self stringForSql:"select count(*) from sqlite_master where name = '%@'", *metaData->tableName] == "0" )
			if ( [self exec:"%@", *metaData->createTableSQL] )
				for ( NSString *idx in *metaData->indexes )
					if ( ![self exec:idx] )
						OOWarn( @"tableMetaDataForClass: Error creating index: %@", idx );
		
		tableMetaDataByClassName[className] = metaData;
	}
	
	return metaData;
}

@end

#pragma mark OOAdaptor - implements all access to a particular database 

@implementation OOAdaptor 

/**
 Connect to/create sqlite3 database
 */

- (OOAdaptor *)initPath:(const OOString &)path database:(OODatabase *)database {
	owner = database;
	if ( (owner->errcode = sqlite3_open( path, &db )) != SQLITE_OK ) {
		OOWarn( @"initPath: Error opening database at path: %@", *path );
		return nil;
	}
	return self;
}

/**
 Prepare a sql statement after which values can be bound and results returned.
 */

- (BOOL)prepare:(const OOString &)sql {
	if ( (owner->errcode = sqlite3_prepare_v2( db, owner->lastSQL = sql, -1, &stmt, 0 )) != SQLITE_OK )
		OOWarn(@"prepare: Could not prepare sql: %@ - %s", *owner->lastSQL, owner->errmsg = (char *)sqlite3_errmsg( db ) );
	return owner->errcode == SQLITE_OK;
}

- (int)bindValue:(id)value asParameter:(int)pno {
#ifdef OODEBUG_BIND
	NSLog( @"bindValue:bindValue: bind parameter #%d as: %@", pno, value );
#endif
	if ( !value || value == (id)kCFNull )
		return sqlite3_bind_null( stmt, pno );
#if OOSQL_THREAD_SAFE_BUT_USES_MORE_MEMORY
	else if ( [value isKindOfClass:[NSString class]] )
		return sqlite3_bind_text( stmt, pno, [value UTF8String], -1, SQLITE_STATIC );
#else
	else if ( [value isKindOfClass:[NSString class]] ) {
		int len = [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
		struct _str_link *str = (struct _str_link *)malloc( sizeof *str + len );
		str->next = strs;
		strs = str;
		[value getCString:str->str maxLength:len+1 encoding:NSUTF8StringEncoding];
		return sqlite3_bind_text( stmt, pno, str->str, len, SQLITE_STATIC );
	}
#endif
	else if ( [value isKindOfClass:[NSData class]] )
		return sqlite3_bind_blob( stmt, pno, [value bytes], [value length], SQLITE_STATIC );			
	
	const char *type = [value objCType];
	if ( type )
		switch ( type[0] ) {
			case 'c': case 's': case 'i': case 'l':
			case 'C': case 'S': case 'I': case 'L':
				return sqlite3_bind_int( stmt, pno, [value intValue] );
			case 'q': case 'Q':
				return sqlite3_bind_int64( stmt, pno, [value longLongValue] );
			case 'f': case 'd':
				return sqlite3_bind_double( stmt, pno, [value doubleValue] );
		}
	
	OOWarn( @"bindValue:bindValue: Undefined type in bind of parameter #%d: %s, value: %@", pno, type, value );
	return -1;
}

/**
 Bind parameters from a prepared SQL statement. The "objCType" method is used to determine the type to bind.
 */

- (BOOL)bindCols:(const OOStringArray &)columns values:(const OODictionary<NSValue *> &)values startingAt:(int)pno bindNulls:(BOOL)bindNulls {
	int errcode;
	for ( NSString *name in *columns )
		if ( bindNulls || *values[name] != (id)kCFNull )
			if ( (errcode = [self bindValue:values[name] asParameter:pno++]) != SQLITE_OK )
				OOWarn( @"bindCols:... Bind failed column: %@ - %s", name, owner->errmsg = (char *)sqlite3_errmsg( db ), owner->errcode = errcode );
	return owner->errcode == SQLITE_OK;
}

/**
 Return a dictionary containing the values for a row returned by the database from a select.
 These values need to be decoded using a classes metadata to set the ivar values later.
 */

- (OODictionary<NSValue *>)valuesForNextRow {
	int ncols = sqlite3_column_count( stmt );
	OODictionary<NSValue *> values;
	
	for ( int i=0 ; i<ncols ; i++ ) {
		OOString name = sqlite3_column_name( stmt, i );
		id value = nil;
		
		switch ( sqlite3_column_type( stmt, i ) ) {
			case SQLITE_NULL:
				value = (id)kCFNull;
				break;
			case SQLITE_INTEGER:
				value = [[NSNumber alloc] initWithLongLong:sqlite3_column_int64( stmt, i )]; 
				break;
			case SQLITE_FLOAT:
				value = [[NSNumber alloc] initWithDouble:sqlite3_column_double( stmt, i )]; 
				break;
			case SQLITE_TEXT: {
				const unsigned char *bytes = sqlite3_column_text( stmt, i );
				value = [[NSMutableString alloc] initWithBytes:bytes
														length:sqlite3_column_bytes( stmt, i) 
													  encoding:NSUTF8StringEncoding];
			}
				break;
			case SQLITE_BLOB: {
				const void *bytes = sqlite3_column_blob( stmt, i );
				value = [[NSData alloc] initWithBytes:bytes length:sqlite3_column_bytes( stmt, i )];
			}
				break;
			default:
				OOWarn( @"valuesForNextRow: Invalid type on bind of ivar %@: %d", *name, sqlite3_column_type( stmt, i ) );
		}
		
		values[name] = value;
		[value release];
	}
	
	return values;
}

/**
 Create instances of the recordClass to store results from a database select or if the record
 class is not present return a list of dictionary objects with the raw results.
 */

- (OOArray<id>)bindResultsIntoInstancesOfClass:(Class)recordClass metaData:(OOMetaData *)metaData {
	OOArray<id> out;
	BOOL awakeFromDB = [recordClass instancesRespondToSelector:@selector(awakeFromDB)];
	
	while( (owner->errcode = sqlite3_step( stmt )) == SQLITE_ROW ) {
		OODictionary<NSValue *> values = [self valuesForNextRow];
		if ( recordClass ) {
			id record = [[recordClass alloc] init];
			[record setValuesForKeysWithDictionary:[metaData decode:values]];
			
			if ( awakeFromDB )
				[record awakeFromDB];
			
			out += record;
			[record release];
		}
		else
			out += values;
	}
	
	if ( owner->errcode != SQLITE_DONE )
		OOWarn(@"bindResultsIntoInstancesOfClass:metaData: Not done (bind) stmt: %@ - %s", *owner->lastSQL, owner->errmsg = (char *)sqlite3_errmsg( db ) );
	else {
		owner->errcode = SQLITE_OK;
		out.alloc();
	}
	
	while ( strs != NULL ) {
		struct _str_link *next = strs->next;
		free( strs );
		strs = next;
	}
	owner->updateCount = sqlite3_changes( db );
	sqlite3_finalize( stmt );
	return out;
}	

- (sqlite_int64)lastInsertRowID {
	return sqlite3_last_insert_rowid( db );
}

- (void) dealloc {
	sqlite3_close( db );
	[super dealloc];
}

@end

#pragma mark OOMetaData instances represent a table in the database and it's record class

@implementation OOMetaData

static OODictionary<OOMetaData *> metaDataByClass;
static OOMetaData *tableOfTables;

+ (NSString *)ooTableTitle { return @"Table MetaData"; }

+ (OOMetaData *)metaDataForClass:(Class)recordClass {
	if ( !tableOfTables )
		[tableOfTables = [[OOMetaData alloc] initClass:[OOMetaData class]] release];
	OOMetaData *metaData = metaDataByClass[recordClass];
	if ( !metaData )
		[metaData = [[OOMetaData alloc] initClass:recordClass] release];
	return metaData;
}

+ (OOArray<id>)selectRecordsRelatedTo:(id)record {
	return [[OODatabase sharedInstance] tablesRelatedByNaturalJoinFrom:record];
}

- (id)initClass:(Class)aClass {
	recordClass = aClass;
	recordClassName = NSStringFromClass(aClass);
	tableTitle = [recordClass respondsToSelector:@selector(ooTableTitle)] ? [recordClass ooTableTitle] : *recordClassName;
	tableName = [recordClass respondsToSelector:@selector(ooTableName)] ? [recordClass ooTableName] : *recordClassName;

	if ( aClass == [OOMetaData class] ) {
		ivars = columns = outcols = boxed = unbox = "tableTitle tableName recordClassName keyColumns ivars columns outcols";
		return metaDataByClass[recordClass] = self;
	}

	createTableSQL = OOFormat( @"create table %@ (", *tableName );

	OOArray<Class> hierarchy;
	do
		hierarchy += recordClass;
	while ( (recordClass = [recordClass superclass]) && recordClass != [NSObject class] );

	for ( int h=(int)hierarchy-1 ; h>=0 ; h-- ) {
		recordClass = hierarchy[h];
		
		Ivar *ivarInfo = class_copyIvarList( recordClass, NULL );
		if ( !ivarInfo )
			continue;
		
		for ( int in=0 ; ivarInfo && ivarInfo[in] ; in++ ) {
			OOString columnName = (NSMutableString *)[[NSString alloc] initWithUTF8String:ivar_getName( ivarInfo[in] )];
			[*columnName release];
			ivars += columnName;

			OOString type = types[columnName] = ivar_getTypeEncoding( ivarInfo[in] ), dbtype = "";
			
			switch ( type[0] ) {
				case 'c': case 's': case 'i': case 'l':
				case 'C': case 'S': case 'I': case 'L':
				case 'q': case 'Q':
					dbtype = @"int";
					break;
				case 'f': case 'd':
					dbtype = @"real";
					break;
				case '{':
					if( !(type & @"=\"ref\"@\"NS") )
						OOWarn( @"initClass: Invalid structure type for ivar %@ in class %@: %@", *columnName, *recordClassName, *type );
					boxed += columnName;
					if ( ![recordClass instancesRespondToSelector:NSSelectorFromString(columnName)] ) {
						unbox += columnName;
						if ( [[recordClass superclass] instancesRespondToSelector:NSSelectorFromString(columnName)] )
							OOWarn( @"initClass: Superclass of class %@ is providing method for column: %@", *recordClassName, *columnName );
					}
				case '@':
					if ( type & @"NS(Mutable)?String\"" )
						dbtype = @"text";
					else if ( type & @"\"NSDate\"" ) {
						dbtype = @"real";
						dates += columnName;
					}
					else {
						if ( !(type & @"NS(Mutable)?Data\"") )
							archived += columnName;
						blobs += columnName;
						dbtype = @"blob";
					}
					break;
				default:
					OOWarn( @"initClass: Unknown data type '%@' in class %@", *type, *tableName );
					archived += columnName;
					blobs += columnName;
					dbtype = @"blob";
					break;
			}

			if ( dbtype == "text" )
				tocopy += columnName;

			if ( columnName == "rowid" || columnName == "ROWID" || columnName == "OID" || columnName == "_ROWID_" ) {
				outcols += columnName;
				continue;
			}

			createTableSQL += OOFormat(@"%s\n\t%@ %@ /* %@ */", !columns?"":",", *columnName, *dbtype, *type );
			if ( isupper( columnName[0] ) )
				indexes += OOFormat(@"create index %@_%@ on %@ (%@)\n",
									*tableName, *columnName,
									*tableName, *columnName);

			if ( columnName[0] != '_' ) {
				columns += columnName;
				outcols += columnName;
				if ( h==0 )
					joinableColumns += columnName;			
			}
		}
		
		free( ivarInfo );
	}
	
	if ( [recordClass respondsToSelector:@selector(ooTableKey)] )
		createTableSQL += OOFormat( @",\n\tprimary key (%@)", *(keyColumns = [recordClass ooTableKey]) );
	
	if ( [recordClass respondsToSelector:@selector(ooConstraints)] )
		createTableSQL += OOFormat( @",\n\t%@", [recordClass ooConstraints] );
	
	createTableSQL += "\n)\n";
	
	if ( [recordClass respondsToSelector:@selector(ooTableSql)] ) {
		createTableSQL = [recordClass ooTableSql];
		indexes = nil;
	}

	tableOfTables->tablesWithNaturalJoin += recordClassName;
	tablesWithNaturalJoin += recordClassName;
	
	for( Class other in [*metaDataByClass allKeys] ) {
		OOMetaData *otherMetaData = metaDataByClass[other];
		if ( otherMetaData == tableOfTables )
			continue;
		
		if ( [self naturalJoinTo:otherMetaData->joinableColumns] > 0 )
			tablesWithNaturalJoin += otherMetaData->recordClassName;
		if ( [otherMetaData naturalJoinTo:joinableColumns] > 0 )
			otherMetaData->tablesWithNaturalJoin += recordClassName;
	}
	
	return metaDataByClass[recordClass] = self;
}

/**
 Find the columns shared between two classes and that have upper case names (are indexed).
 */

- (OOStringArray)naturalJoinTo:(const OOStringArray &)to {
	OOStringArray commonColumns = columns & to;
	for ( int i=0 ; i<commonColumns ; i++ )
		if ( islower( (*commonColumns[i])[0] ) )
			~commonColumns[i--];
	return commonColumns;
}

/**
 Encode values ready for insertion into the database (convert OOString to NSString etc.)
 */

- (const OODictionary<NSValue *> &)encode:(const OODictionary<NSValue *> &)values {
	for ( NSString *key in *unbox ) {
		id value = (id)[*values[key] pointerValue];
		values[key] = value ? value : (id)kCFNull;
	}
	for ( NSString *key in *dates )
		values[key] = [NSNumber numberWithDouble:[values[key] timeIntervalSince1970]];
	for ( NSString *key in *archived )
		values[key] = (NSValue *)[NSKeyedArchiver archivedDataWithRootObject:values[key]];
	return values;
}

/**
 Decode values taken from the database for use in [record setValuesForKeysWithDictionary:values];
 */

- (const OODictionary<NSValue *> &)decode:(const OODictionary<NSValue *> &)values {
	id value;
	for ( NSString *key in *archived ) 
		if ( value = values[key] )
			values[key] = [NSKeyedUnarchiver unarchiveObjectWithData:(NSData *)value];
	for ( NSString *key in *dates )
		if ( value = values[key] )
			values[key] = [NSDate dateWithTimeIntervalSince1970:[value doubleValue]];
	for ( NSString *key in *boxed ) {
		if ( value = values[key] ) {
			value = value != (id)kCFNull ? [value retain] : nil;
			[values[key] = [[NSValue alloc] initWithBytes:&value objCType:@encode(id)] release];
		}
	}
	return values;
}

/**
 import values for a list of nodes selected from an XML document.
 */

+ (OOArray<id>)import:(const OOArray<OODictionary<OOString> > &)nodes intoClass:(Class)recordClass {
	OOMetaData *metaData = [self metaDataForClass:recordClass];
	OOArray<id> out;
	
	for ( NSMutableDictionary *dict in *nodes ) {
		OODictionary<OOString> node = dict, values;
		for ( NSString *ivar in *metaData->columns )
			values[ivar] = node[ivar];
		
		id record = [[recordClass alloc] init];
		[record setValuesForKeysWithDictionary:[metaData decode:values]];
		out += record;
		[record release];
	}
	
	return out;
}

/**
 Convert a string taken from a flat file into record instances which can be inserted into the database.
 */

+ (OOArray<id>)import:(const OOString &)string intoClass:(Class)recordClass delimiter:(const OOString &)delim {
	OOMetaData *metaData = [self metaDataForClass:recordClass];
	// remove escaped newlines then split by newline
	OOStringArray lines = (string - "\\\n") / "\n";
	lines--; // pop last empty line
	
	OOArray<id> out;
	for ( int l=0 ; l<lines ; l++ ) {
		OODictionary<NSString *> values;
		values[metaData->columns] = *lines[l] / delim;
		
		// empty columns are taken as null values
		for ( NSString *key in *metaData->columns )
			if ( [*values[key] isEqualToString:@""] )
				values[key] = (id)kCFNull;
		
		// convert description strings to NSData
		for ( NSString *key in *metaData->blobs )
			[values[key] = [[NSData alloc] initWithDescription:values[key]] release];
		
		id record = [[recordClass alloc] init];
		[record setValuesForKeysWithDictionary:[metaData decode:values]];
		out += record;
		[record release];
	}
	
	return out;
}

/**
 Convert a set of records selected from the database into a string which can be saved to disk.
 */

+ (OOString)export:(const OOArray<id> &)array delimiter:(const OOString &)delim {
	OOMetaData *metaData = nil;
	OOString out;
	
	for ( id record in *array ) {
		if ( !metaData )
			metaData = [record isKindOfClass:[NSDictionary class]] ?
			(id)kCFNull : [self metaDataForClass:[record class]];
		
		OODictionary<NSValue *> values = metaData == (id)kCFNull ? record : 
		*[metaData encode:[record dictionaryWithValuesForKeys:metaData->columns]];
		
		OOStringArray line;
		NSString *blank = @"";
		for ( NSString *key in *metaData->columns )
			line += *values[key] != (id)kCFNull ? [values[key] stringValue] : blank;
		
		out += line/delim+"\n";
	}
	
	return out;
}

/**
 Bind a record to a view containing elements which are to display values from the record.
 The ivar number is selected by the subview's tag value and it's ".text" property if set to
 the value returned record value "stringValue" for the ivar. Supports images stored as
 NSData objects, UISwitches bound to boolean valuea and UITextField for alll other values.
 */

+ (void)bindRecord:(id)record toView:(UIView *)view delegate:(id)delegate {
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
	OOMetaData *metaData = [self metaDataForClass:[record class]];
	OODictionary<NSValue *> values = [metaData encode:[record dictionaryWithValuesForKeys:metaData->ivars]];

	for ( int i=0 ; i<metaData->ivars ; i++ ) {
		UILabel *label = (UILabel *)[view viewWithTag:1+i];
		id value = values[metaData->ivars[i]];
		
		if ( [label isKindOfClass:[UIImageView class]] )
			((UIImageView *)label).image = value != (id)kCFNull ? [UIImage imageWithData:(NSData *)value] : nil;
		else if ( [label isKindOfClass:[UISwitch class]] ) {
			UISwitch *uiSwitch = (UISwitch *)label;
			uiSwitch.on = value != (id)kCFNull ? [value charValue] : 0;
			if ( delegate )
				[uiSwitch addTarget:delegate action:@selector(valueChanged:) forControlEvents:UIControlEventValueChanged];
		}
		else if ( [label isKindOfClass:[UIWebView class]] )
			[(UIWebView *)label loadHTMLString:value != (id)kCFNull ? value : @"" baseURL:nil];
		else if ( label ) {
			label.text = value != (id)kCFNull ? [value stringValue] : @"";
			if ( [label isKindOfClass:[UITextView class]] ) {
				[(UITextView *)label setContentOffset:CGPointMake(0,0) animated:NO];
				[(UITextView *)label scrollRangeToVisible:NSMakeRange(0,0)];
			}
		}
		
		if ( [label respondsToSelector:@selector(delegate)] )
			((UITextField *)label).delegate = delegate;
		label.hidden = NO;
		
		if ( label = (UILabel *)[view viewWithTag:-1-i] ) {
			label.text = **metaData->ivars[i];
			label.hidden = NO;
		}
	}
	
	UIView *subView;
	for ( int i=metaData->ivars ; subView = [view viewWithTag:1+i] ; i++ ) {
		subView.hidden = YES;
		if ( subView = [view viewWithTag:-1-i] )
			subView.hidden =YES;
	}
#endif
}

/**
 When the delegate method fires this method should be called to update
 the record with the modified value before updating the database.
 */

+ (void)updateRecord:(id)record fromView:(UIView *)view {
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
	if ( view.tag > 0 && [view respondsToSelector:@selector(text)] ) {
		OOMetaData *metaData = [self metaDataForClass:[record class]];
		NSString *name = *metaData->ivars[view.tag-1];
		OOString type = metaData->types[name];
		id value = [((UITextField *)view).text retain];
		
		if ( type[0] == '{' ) {
			value = [[NSValue alloc] initWithBytes:&value objCType:@encode(id)];
			[(id)[[record valueForKey:name] pointerValue] release];
		}
		
		[record setValue:value forKey:name];
		[value release];
	}
	for ( UIView *subview in [view subviews] )
		[self updateRecord:record fromView:subview];
#endif
}

@end

@implementation NSData(OOExtras)

static int unhex ( unsigned char ch ) {
	return ch >= 'a' ? 10 + ch - 'a'  : ch >= 'A' ? 10 + ch - 'A' : ch - '0';
}

- initWithDescription:(NSString *)description {
	int len = [description length]/2, lin = [description lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
	char *bytes = (char *)malloc( len ), *optr = bytes, *hex = (char *)malloc( lin+1 );
	[description getCString:hex maxLength:lin+1 encoding:NSUTF8StringEncoding];

	for ( char *iptr = hex ; *iptr ; iptr+=2 ) {
		if ( *iptr == '<' || *iptr == ' ' || *iptr == '>' )
			iptr--;
		else
			*optr++ = unhex( *iptr )*16 + unhex( *(iptr+1) );
	}
	
	free( hex );
	return [self initWithBytesNoCopy:bytes length:optr-bytes freeWhenDone:YES];
}

- (NSString *)stringValue { return [self description]; }

@end

@interface NSString(OOExtras)
@end
@implementation NSString(OOExtras)
- (char)charValue { return [self intValue]; }
- (char)shortValue { return [self intValue]; }
- (NSString *)stringValue { return self; }
@end

@interface NSArray(OOExtras)
@end
@implementation NSArray(OOExtras)
- (NSString *)stringValue { 
	static OOReplace reformat( "/(\\s)\\s+|^\\(|\\)$|\"/$1/" );
	return &([self description] | reformat);
}
@end

@interface NSDictionary(OOExtras)
@end
@implementation NSDictionary(OOExtras)
- (NSString *)stringValue { 
	static OOReplace reformat( "/(\\s)\\s+|^\\{|\\}$|\"/$1/" );
	return &([self description] | reformat);
}
@end

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
@interface UISwitch(OOExtras)
@end
@implementation UISwitch(OOExtras)
- (NSString *)text { return self.on ? @"1" : @"0"; }
@end
#endif

