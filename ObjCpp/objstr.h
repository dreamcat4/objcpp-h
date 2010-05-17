/*
 *  objstr.h - NSString based string class with operators
 *  ========
 *
 *  Created by John Holdsworth on 01/04/2009.
 *  Copyright 2009 John Holdsworth.
 *
 *  $Id: //depot/2.2/ObjCpp/objstr.h#1 $
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

#import "objcpp.h"

/*====================================================================================*/
/*============================= String classes =======================================*/

/**
 Internal class representing a susbscript operation into a string to access or assign
 to individual characters or ranges
 
 Usage:
 <pre>
 OOString str <<= @"JOHN";
 if ( str[1] != 'O' )
 str[1] = 'O';
 </pre>
 */

class OOStringSub {
	friend class OOString;
	NSMutableString *str;
	NSRange idx;
	oo_inline OOStringSub( NSMutableString *ref, int sub ) {
		str = ref;
		idx = NSMakeRange( sub < 0 ? [str length]+sub : sub, 1 );
	}
	oo_inline OOStringSub( NSMutableString *ref, const NSRange &sub ) {
		str = ref;
		idx = sub;
	}
public:
#if 0
	oo_inline BOOL isupper() {
		return isupper( (int)**this );
	}
	oo_inline BOOL islower() {
		return islower( (int)**this );
	}
#endif
	oo_inline operator unichar () const {
		return **this;
	}
	oo_inline unichar operator * () const {
		return [str characterAtIndex:idx.location];
	}
	oo_inline operator NSString * () const {
		return [str substringWithRange:idx];
	}
	oo_inline OOStringSub & operator = ( unichar val ) {
		[str replaceCharactersInRange:idx withString:[NSString stringWithFormat:@"%c", val]];
		return *this;
	}
	oo_inline OOStringSub & operator = ( NSString *val ) {
		[str replaceCharactersInRange:idx withString:val];
		return *this;
	}
	oo_inline OOStringSub & operator = ( const char *val ) {
		return operator = ( [NSString stringWithUTF8String:val] );
	}
};

/**
 Internal class representing subscript by string which performs a search into the
 string. Assigning to for example str[@"BARRY"] = @"BAZ" will change all occurances
 of "BARRY" in the string to "BAZ".
 
 Usage:
 <pre>
 OOString str <<= @"BARRY is great";
 str[@"great"] = @"an egotist";
 </pre>
 */

class OOStringSearch {
	friend class OOString;
	NSMutableString *str;
	NSString *idx;
	oo_inline OOStringSearch( NSMutableString *ref,  NSString *sub ) {
		str = ref;
		idx = sub;
	}
public:
	oo_inline operator NSRange () const {
		return [str rangeOfString:idx];
	}
	oo_inline operator NSUInteger () const {
		return [str rangeOfString:idx].location;
	}
	oo_inline OOStringSearch & operator = ( NSString *replacement ) {
		[str setString:[str stringByReplacingOccurrencesOfString:idx withString:replacement]];
		return *this;
	}
	oo_inline OOStringSearch & operator = ( const char *replacement ) {
		*this = [NSString stringWithUTF8String:replacement];
		return *this;
	}
	oo_inline BOOL operator ! () const {
		return !str || [str rangeOfString:idx].location == NSNotFound;
	}
};

class OOTmpStr;

/**
 A string class wrapping around NSString with all the usual operators including subscript.
 OOStringArray is \#defined as OOArray<OOString> and OOStringDict is \#defined as OODictionary<OString>
 for convenience.
 
 Operators:
 <table cellspacing=0><tr><th>operator<th>inplace<th>binary<th>arguments
 <tr><td>assign<td>=<td>&nbsp;<td>String or NSString
 <tr><td>copy<td>&lt;&lt;=<td>&nbsp;<td>String or NSString
 <tr><td>append<td>+=<td>+<td>string or number
 <tr><td>remove<td>-=<td>-<td>string
 <tr><td>repeat<td>*=<td>*<td>count
 <tr><td>split<td>&nbsp;<td>/<td>string
 <tr><td>find<td><td>&amp;<td>pattern
 <tr><td>parse<td><td>^<td>pattern
 <tr><td>replace<td>|=<td>|<td>replace string = "/pat/with/"
 <tr><td>subscript<td>&nbsp;<td>[]<td>character number
 <tr><td>search<td>&nbsp;<td>[]<td>string
 </table>
 
 Usage:
 <pre>
 OOString str <<= @"The time is ";
 NSLog( "%@", *(str+ctime()) );
 </pre>
 */

class OOString : public OOReference<NSMutableString *> {
public:
	oo_inline OOString() {}
	///oo_inline OOString( id val ) { set( val ); }
	oo_inline OOString( CFNullRef obj ) { set( (id)obj ); }
	oo_inline OOString( const OOString &str ) { *this = str; }
	oo_inline OOString( const OOStringSub &sub ) { *this = sub; }
	oo_inline OOString( const OOArraySub<OOString> &sub ) { *this = sub; }
	oo_inline OOString( const OODictionarySub<OOString> &sub ) { *this = sub; }
	oo_inline OOString( int nilOrCapacity ) { *this = nilOrCapacity; }
	oo_inline OOString( NSMutableString *str ) { *this = str; }
	oo_inline OOString( NSString *str ) { *this = str; }
	oo_inline OOString( double val ) { *this += val; }
	oo_inline OOString( const char *val ) { 
		[set( [[NSMutableString alloc] initWithUTF8String:val] ) release];
	}
	oo_inline OOString( const char *val, int len, int encoding = NSUTF8StringEncoding ) { 
		[set( [[NSMutableString alloc] initWithBytes:val length:len encoding:encoding] ) release];
	}
	oo_inline OOString( const OOStringArray &val ) {
		*this = val.join();
	}
	oo_inline OOData utf8Data() const {
		int len = [get() lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
		char *bytes = (char *)malloc( len+1 );
		[get() getCString:bytes maxLength:len+1 encoding:NSUTF8StringEncoding];
		OOData out = [[NSData alloc] initWithBytesNoCopy:bytes length:len freeWhenDone:YES];
		[*out release];
		return out;
	}
	
	// basic operators
	oo_inline NSMutableString *operator & () const { return autoget(); }
	oo_inline operator const char * () const { return [get() UTF8String]; }
	oo_inline operator double () const { return [get() doubleValue]; }
	oo_inline operator OOData () const { return utf8Data(); }

	oo_inline OOString capitals () { return [get() capitalizedString]; }
	oo_inline OOString toupper () { return [get() uppercaseString]; }
	oo_inline OOString tolower () { return [get() lowercaseString]; }
	
	// assignment
	oo_inline OOString &operator = ( NSMutableString *val ) { set( val ); return *this; }
	oo_inline OOString &operator = ( NSString *val ) { OOCopyImmutable( val ); return *this; }
	oo_inline OOString &operator = ( int nilOrCapacity ) { set( nilOrCapacity ); return *this; }
	oo_inline OOString &operator = ( const char *val ) { set( OOString(val).get() ); return *this; }
	oo_inline OOString &operator = ( const OOString &val ) { set( val.get() ); return *this; }
	oo_inline OOString &operator = ( const OOStringSub &val ) { OOCopyImmutable( val ); return *this; }
	oo_inline OOString &operator = ( const OOArraySub<OOString> &val ) { set( val.get() ); return *this; }
	oo_inline OOString &operator = ( const OODictionarySub<OOString> &val ) { set( val.get() ); return *this; }
	oo_inline OOString &operator = ( const OOTmpStr &val );

	// inplace operators - append, remove, repeat
	oo_inline OOString &operator += ( id val ) { [alloc() appendString:[val description]]; return *this; }
	oo_inline OOString &operator += ( int val ) { [alloc() appendFormat:@"%d", val]; return *this; }
	oo_inline OOString &operator += ( double val ) { [alloc() appendFormat:@"%f", val]; return *this; }
	oo_inline OOString &operator += ( const char *val ) { [alloc() appendString:OOString( val ).get()]; return *this; }
	oo_inline OOString &operator += ( const NSMutableString *str ) { [alloc() appendString:str]; return *this; }
	oo_inline OOString &operator += ( const NSString *str ) { [alloc() appendString:str]; return *this; }
	oo_inline OOString &operator += ( const OOString &str ) { [alloc() appendString:str.get()]; return *this; }
	oo_inline OOString &operator += ( const OOArraySub<OOString> &str ) { [alloc() appendString:str.get()]; return *this; }
	oo_inline OOString &operator += ( const OODictionarySub<OOString> &str ) { [alloc() appendString:str.get()]; return *this; }

	oo_inline OOString &operator -= ( const OOString &str ) { alloc(); (*this)[str] = @""; return *this; }
	oo_inline OOString &operator *= ( NSUInteger count ) {
		NSString *str = [[get() copy] autorelease];
		*this = "";
		for ( int i=0 ; i<count ; i++ )
			*this += str;
		return *this;
	}
	
	// string comparison
	oo_inline BOOL operator == ( const char *str ) const { return [get() isEqualToString:OOString(str).get()]; }
	oo_inline BOOL operator != ( const char *str ) const { return !operator == ( str ); }
	oo_inline BOOL operator <  ( const char *str ) const { return [get() caseInsensitiveCompare:OOString(str).get()] == NSOrderedAscending; }
	oo_inline BOOL operator >= ( const char *str ) const { return !operator < ( str ); }
	oo_inline BOOL operator >  ( const char *str ) const { return [get() caseInsensitiveCompare:OOString(str).get()] == NSOrderedDescending; }
	oo_inline BOOL operator <= ( const char *str ) const { return !operator > ( str ); }
	oo_inline BOOL operator == ( NSString *str ) const { return [get() isEqualToString:str]; }
	oo_inline BOOL operator != ( NSString *str ) const { return !operator == ( str ); }
	oo_inline BOOL operator <  ( NSString *str ) const { return [get() caseInsensitiveCompare:str] == NSOrderedAscending; }
	oo_inline BOOL operator >= ( NSString *str ) const { return !operator < ( str ); }
	oo_inline BOOL operator >  ( NSString *str ) const { return [get() caseInsensitiveCompare:str] == NSOrderedDescending; }
	oo_inline BOOL operator <= ( NSString *str ) const { return !operator > ( str ); }
	oo_inline BOOL operator == ( NSMutableString *str ) const { return [get() isEqualToString:str]; }
	oo_inline BOOL operator != ( NSMutableString *str ) const { return !operator == ( str ); }
	oo_inline BOOL operator <  ( NSMutableString *str ) const { return [get() caseInsensitiveCompare:str] == NSOrderedAscending; }
	oo_inline BOOL operator >= ( NSMutableString *str ) const { return !operator < ( str ); }
	oo_inline BOOL operator >  ( NSMutableString *str ) const { return [get() caseInsensitiveCompare:str] == NSOrderedDescending; }
	oo_inline BOOL operator <= ( NSMutableString *str ) const { return !operator > ( str ); }
	oo_inline BOOL operator == ( const OOString &str ) const { return [get() isEqualToString:str.get()]; }
	oo_inline BOOL operator != ( const OOString &str ) const { return !operator == ( str ); }
	oo_inline BOOL operator <  ( const OOString &str ) const { return [get() caseInsensitiveCompare:str.get()] == NSOrderedAscending; }
	oo_inline BOOL operator >= ( const OOString &str ) const { return !operator < ( str ); }
	oo_inline BOOL operator >  ( const OOString &str ) const { return [get() caseInsensitiveCompare:str.get()] == NSOrderedDescending; }
	oo_inline BOOL operator <= ( const OOString &str ) const { return !operator > ( str ); }
	
	// append string
	oo_inline OOString operator + ( int val ) const {
		return [noalloc() stringByAppendingFormat:@"%d", val];
	}
	oo_inline OOString operator + ( float val ) const {
		return [noalloc() stringByAppendingFormat:@"%f", val];
	}	
	oo_inline OOString operator + ( double val ) const {
		return [noalloc() stringByAppendingFormat:@"%f", val];
	}	
	oo_inline OOString operator + ( const OOString &str ) const {
		return operator + ( str.get() );
	}	
	oo_inline OOString operator + ( const OOArraySub<OOString> &str ) const {
		return operator + ( str.get() );
	}	
	oo_inline OOString operator + ( const OODictionarySub<OOString> &str ) const {
		return operator + ( str.get() );
	}	
	oo_inline OOString operator + ( const OOArraySub<NSString *> &str ) const {
		return operator + ( str.get() );
	}	
	oo_inline OOString operator + ( const OODictionarySub<NSString *> &str ) const {
		return operator + ( str.get() );
	}	
	oo_inline OOString operator + ( NSString *str ) const {
		return [noalloc() stringByAppendingString:str?str:@"<nil>"];
	}
	
	// remove string
	oo_inline OOString operator - ( const char *val ) const {
		return operator - ( OOString( val ).get() );
	}	
	oo_inline OOString operator - ( const OOString &str ) const {
		return operator - ( str.get() );
	}	
	oo_inline OOString operator - ( NSString *str )const {
		return [noalloc() stringByReplacingOccurrencesOfString:str withString:@""];
	}
	
	// split by str
	oo_inline OOStringArray operator / ( const OOString &sep ) const {
		return [noalloc() componentsSeparatedByString:sep];
	}
	
	// index into string by character number or perform search
	oo_inline OOStringSub operator [] ( int sub ) const { return OOStringSub( get(), sub ); }
	oo_inline OOStringSub operator [] ( const NSRange &sub ) const { return OOStringSub( get(), sub ); }
	oo_inline OOStringSearch operator [] ( const OOString &sub ) const { return OOStringSearch( get(), sub ); }

	// for pattern matching - see below...
};

// replicate string 
inline OOString operator * ( const OOString &str, int count ) {
	OOString out = "";
	for ( int i=0 ; i<count ; i++ )
		out += str;
	return out;
}


template <typename ETYPE>
inline OOString OOArray<ETYPE>::join( const OOString &sep ) const { 
	OOString out = "";
	for ( int i=0 ; i<*this ; i++ ) {
		if ( i )
			out += sep;
		out += (*this)[i];
	}
	return out;
}

template <typename ETYPE>
inline OOString operator / ( const OOArray<ETYPE> &left, const OOString &sep ) {
	return left.join( sep );
}

template <typename ETYPE>
inline OOArray<ETYPE> &OOArray<ETYPE>::operator = ( const char *val ) { *this = *(OOString(val) / @" "); return *this; }

template <typename ETYPE>
inline OODictionary<ETYPE> &OODictionary<ETYPE>::operator = ( const char *val ) { *this = *(OOString(val) / @" "); return *this; }

template <typename ETYPE>
inline OODictionarySub<ETYPE> OODictionary<ETYPE>::operator [] ( const OOArraySub<OOString> &sub ) const {
	return OODictionarySub<ETYPE>( this, sub.get() );
}

template <typename ETYPE>
inline OODictionarySub<ETYPE> OODictionary<ETYPE>::operator [] ( const OODictionarySub<OOString> &sub ) const {
	return OODictionarySub<ETYPE>( this, sub.get() );
}

inline OOString operator + ( const char *left, const OOString &right ) { return OOString( left )+right; }
inline OOString operator + ( const char *left, const OOArraySub<OOString> &right ) { return OOString( left )+*right; }
inline OOString operator + ( const char *left, const OODictionarySub<OOString> &right ) { return OOString( left )+*right; }

inline OOString operator || ( const OOString &left, const OOString &right ) { return !left ? right : left; }
inline OOString operator || ( const OOString &left, const char *right ) { return left || OOString( right ); }

oo_inline static NSMutableString *OOFormat( const OOString &format, ... ) {
	va_list argp;
	va_start(argp, format);
	NSString *str = [[NSMutableString alloc] initWithFormat:format arguments:argp];
	va_end( argp );	
	return [str autorelease];
}

/*=================================================================================*/
/*================================ Pattern matching classes =======================*/

extern "C" {
#include <regex.h>
}

/**
 A class to represent a patten matching operations against a string.
 
 Usage:
 <pre>
OOStringArray words = OOPattern( @"\\w+" ).match();
 </pre>
 */

class OOPattern {
	OOString pat;
public:
	regex_t *regex;
	
	oo_inline OOPattern() {}
	oo_inline OOPattern( const OOString &patin, int flags = REG_EXTENDED ) { init( patin, flags ); }

	oo_inline void init( const OOString &patin, int flags );
	oo_inline int exec( const char *input, int matchmax = 0, regmatch_t matches[] = NULL, int eflags = 0 ) const {
		int error = regexec( regex, input, matchmax, matches, eflags );
		if ( error && error != REG_NOMATCH ) {
			char errbuff[1024];
			regerror( error, regex, errbuff, sizeof errbuff );
			OOWarn( @"OOPattern::exec() Regex match error: %s, in pattern'%@'", errbuff, *pat );
		}
		return error == 0;
	}

	oo_inline int find( const char *input, int matchmax, regmatch_t matches[] ) const {
		int nmatches = 0;
		for ( int pos = 0 ; exec( input+pos, matchmax-nmatches, matches+nmatches ) ; nmatches++ ) {
			matches[nmatches].rm_so += pos;
			pos = matches[nmatches].rm_eo += pos;
		}
		return nmatches;
	}
	oo_inline OOStringArray split( const OOString &str ) const {
		regmatch_t matches[100];
		const char *input = str;
		OOStringArray out;
		
		for ( ; exec( input, sizeof matches/sizeof matches[0], matches ) ; input += matches[0].rm_eo )
			out += OOString( input, matches[0].rm_so );	
		out += OOString( input );

		return out;
	}
	oo_inline OOStringArray match( const OOString &str ) const {
		regmatch_t matches[100];
		OOStringArray out;
		
		for ( const char *input = str ; exec( input, sizeof matches/sizeof matches[0], matches ) ; input += matches[0].rm_eo )
			out += OOString( input+matches[0].rm_so, matches[0].rm_eo-matches[0].rm_so );	

		return out;
	}
	oo_inline OOStringArray parse( const OOString &str ) const {
		regmatch_t matches[100];
		const char *input = str;
		OOStringArray out;
		
		if ( exec( input, sizeof matches/sizeof matches[0], matches ) )
			for ( int i=0 ; i<=regex->re_nsub ; i++ )
				out += matches[i].rm_so == -1 ? OOString( kCFNull ) :
				OOString( input+matches[i].rm_so, matches[i].rm_eo-matches[i].rm_so );

		return out;
	}
};

inline OOStringArray operator & ( const OOString &str, const OOPattern &pattern ) { return pattern.match( str ); }
inline OOStringArray operator & ( const OOString &str, const OOPattern *pattern ) { return pattern->match( str ); }
inline OOStringArray operator & ( const OOString &str, const OOString  &patexpr ) { return str & OOPattern( patexpr ); }

inline OOStringArray operator ^ ( const OOString &str, const OOPattern &pattern ) { return pattern.parse( str ); }
inline OOStringArray operator ^ ( const OOString &str, const OOPattern *pattern ) { return pattern->parse( str ); }
inline OOStringArray operator ^ ( const OOString &str, const OOString  &patexpr ) { return str ^ OOPattern( patexpr ); }

/**
 A class to represent a match and replace operations against a string.
 
 Usage:
 <pre>
 OOString quoted = OOReplace( @"/(\\w+)/'$1'/" ).exec();
 </pre>
 */

class OOReplace {
	regmatch_t dmatches[100];
	OOPattern pattern;
	OOString replace;
	int dcount;

public:
	oo_inline OOReplace() {}
	oo_inline OOReplace( const OOString &expr ) {
		init( expr );
	}
	oo_inline OOReplace( const OOString &pat, const OOString &rep, int flags ) {
		init( pat, rep, flags );
	}

	oo_inline void init( const OOString &expr ) {
		OOStringArray split = expr / expr[NSMakeRange(0,1)];
		int flags = REG_EXTENDED;
		
		for ( const char *options = *split[3] ; *options ; options++ )
			switch ( *options ) {
				case 'i': flags |= REG_ICASE; break;
				case 'm': flags |= REG_NEWLINE; break;
			}
		
		init( split[1], split[2], flags );
	}
	oo_inline void init( const OOString &pat, const OOString &rep, int flags ) {
		pattern.init( pat, flags );
		replace <<= rep;

		static OOPattern *dollarPattern;
		if ( !dollarPattern )
			dollarPattern = new OOPattern( "\\\\\\$|\\$[[:digit:]]+|\\${[[:digit:]]+}", REG_EXTENDED );

		dcount = dollarPattern->find( replace, sizeof dmatches / sizeof dmatches[0], dmatches );
	}

	oo_inline OOString exec( const char *input ) const {
		const char *dstr = replace;
		regmatch_t matches[100];
		OOString out;
		
		for ( ; pattern.exec( input, sizeof matches / sizeof matches[0], matches ) ; input += matches[0].rm_eo ) {
			out += OOString( input, matches[0].rm_so );
			int pos = 0;
			for ( int d=0 ; d<dcount ; d++ ) {
				out += OOString( dstr+pos, dmatches[d].rm_so-pos );
				const char *vptr = dstr+dmatches[d].rm_so+1; 
				int v = atoi( *vptr == '{' ? vptr+1 : vptr );
				out += *vptr == '$' ? OOString( vptr, 1 ) :	OOString( input+matches[v].rm_so, matches[v].rm_eo-matches[v].rm_so );
				pos = dmatches[d].rm_eo;
			}
			out += OOString( dstr+pos );
		}
		
		return out += OOString( input );
	}
};

inline OOString operator | ( const OOString &str, const OOReplace &replace ) { return replace.exec( str ); }
inline OOString operator | ( const OOString &str, const OOReplace *replace ) { return replace->exec( str ); }
inline OOString operator | ( const OOString &str, const OOString  &patrepl ) { return str | OOReplace( patrepl ); }

inline OOString &operator |= ( OOString &str, const OOReplace &replace ) { return str = str | replace; }
inline OOString &operator |= ( OOString &str, const OOReplace *replace ) { return str = str | replace; }
inline OOString &operator |= ( OOString &str, const OOString  &patrepl ) { return str = str | patrepl; }

/**
 A class to represent a C pointer inside an NSValue object for ref counting and
 so it can be stored inside the OSX buffs NSArray, NSDictionary etc.
 
 Usage:
 <pre>
 OOPointer<void *> ptr = malloc(1000);
 void *p = ptr;
 </pre>
 */

template <typename PTYPE>
class OOPointer : public OOReference<NSValue *> {
	PTYPE ptr;
protected:
	oo_inline PTYPE pget() {
		return !*this ? NULL : ptr;
	}
	oo_inline NSValue *pset( NSValue *val ) {
		// recover pointer from NSValue object
		set( val );
		ptr = val == (id)kCFNull ? NULL : (PTYPE)[val  pointerValue];
		return val;
	}
	oo_inline PTYPE pset( PTYPE ptr ) {
		// store pointer in NSValue objects so it can be referrence counted
		[pset( [[NSValue alloc] initWithBytes:&ptr objCType:@encode(PTYPE)] ) release];
		return ptr;
	}
public:
	oo_inline OOPointer() { }
	oo_inline OOPointer( PTYPE ptr ) { *this = ptr; }
	oo_inline OOPointer( NSValue *val ) { *this = val; }
	oo_inline OOPointer( const OOPointer &val ) { *this = val; }
	
	oo_inline operator PTYPE () { return pget(); }
	oo_inline PTYPE operator * () { return pget(); }
	
	oo_inline OOPointer &operator = ( PTYPE ptr ) { pset( ptr ); return *this;	}
	oo_inline OOPointer &operator = ( NSValue *val ) { pset( val ); return *this; }
	oo_inline OOPointer &operator = ( const OOPointer &val ) { pset( val.get() ); return *this; }
};


/**
 Initialisation for OOPattern class. Replaces \[wWdWsS] with character classes 
for compatability with perl. All compiled reguar expressions are cached.
 */

inline void OOPattern::init( const OOString &patin, int flags ) {
	pat <<= patin;
	
	static OODictionary<OOPointer<regex_t *> > cache;
	if ( !!cache[patin][flags] ) {
		regex = *cache[patin][flags];
		return;
	}
	
	static BOOL patfix;
	if ( !patfix ) {
		static OOReplace *patternShortcuts;
		static const char *shortcuts[] = {
			"w/$1[[:alnum:]]/", "W/$1[^[:alnum:]]/",
			"d/$1[[:digit:]]/", "D/$1[^[:digit:]]/",
			"s/$1[[:space:]]/", "S/$1[^[:space:]]/"};
		int savefix = patfix, nfixes = sizeof shortcuts/sizeof *shortcuts;
		
		if ( !patternShortcuts ) {
			patfix = YES;
			patternShortcuts = new OOReplace[nfixes];
			for ( int i=0 ; i<nfixes ; i++ )
				patternShortcuts[i].init( OOString("/(^|[^\\\\](\\\\\\\\)*)\\\\")+shortcuts[i] );
			patfix = savefix;
		}
		
		for ( int i=0 ; i<nfixes ; i++ )
			pat |= patternShortcuts[i];
	}
	
	regex = new regex_t;

	int error = regcomp( regex, pat, flags );
	if ( error ) {
		char errbuff[1024];
		regerror( error, regex, errbuff, sizeof errbuff );
		OOWarn( @"OOPattern::init() Regex compilation error: %s, in pattern'%@'", errbuff, *pat );
		delete regex;
	}
	else
		cache[patin][flags] = regex;	
}

/*=================================================================================*/
/*================================ Utility classes ================================*/

/**
 Network request on which you can POST data or set HTP header values.
 */

class OORequest : public OOReference<NSMutableURLRequest *> {
public:
	NSURLResponse *theResponse;
	NSError *error;

	oo_inline OORequest( NSURL *url, 
						NSURLRequestCachePolicy cachePolicy = NSURLRequestUseProtocolCachePolicy, 
						NSTimeInterval timeoutInterval = 60. ) {
		[set( [[NSMutableURLRequest alloc] initWithURL:url cachePolicy:cachePolicy timeoutInterval:timeoutInterval] ) release];
	}
	oo_inline OORequest( OOString url ) {
		[set( [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:url]] ) release];
		
	}
	oo_inline OORequest( const OORequest &req ) {
		set( req.get() );
	}

	oo_inline OORequest &property( const OOString &name, const OOString &value ) {
		[get() setValue:value forHTTPHeaderField:name];
		return *this;
	}
	oo_inline OORequest &post( NSData *data ) {
		[get() setHTTPMethod:@"POST"];
		property( @"Content-Encoding", @"application/x-www-form-urlencoded" );
		property( @"Content-Length", OOFormat(@"%d", [data length] ) );
		[get() setHTTPBody:data];
		return *this;
	}
	oo_inline OORequest &post( const OOString &str, NSStringEncoding encoding = NSUTF8StringEncoding ) {
		post( [urlEncode( str ) dataUsingEncoding:encoding allowLossyConversion:YES] );	
		return *this;
	}

	static oo_inline NSString *urlEncode( NSString *text ) {
		NSMutableString *encoded = [NSMutableString string];
		for ( const char *iptr = [text UTF8String] ; iptr && *iptr ; iptr++ )
			if ( *iptr > 0 )
				[encoded appendFormat:@"%c", *iptr];
			else
				[encoded appendFormat:@"%%%02x", *iptr&0xff];
		
		return encoded;
	}
	
	OOReference<NSData *> data() {
		return [NSURLConnection sendSynchronousRequest:get() returningResponse:&theResponse error:&error];
	}
	OOString string() {
		NSData *data = [NSURLConnection sendSynchronousRequest:get() returningResponse:&theResponse error:&error];
		CFStringRef textEncoding = (CFStringRef)[theResponse textEncodingName];
		CFStringEncoding coreEncoding = textEncoding ? CFStringConvertIANACharSetNameToEncoding( textEncoding ) : kCFStringEncodingUTF8;
		NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding( coreEncoding );
		OOString out = [[NSMutableString alloc] initWithData:data encoding:encoding];
		[*out release];
		return out;
	}
};

/**
 OOURL to initialise strings from the network or files.
 */

class OOURL : public OOReference<NSURL *> {
public:
	NSError *error;

	oo_inline OOURL( NSString *url ) {
		setURL( url );
	}
	oo_inline OOURL( const OOString &url = nil ) {
		setURL( url );
	}
	oo_inline void setURL( NSString *url ) {
		if ( url )
			[set( [[NSURL alloc] initWithString:url] ) release];
	}
	oo_inline OOString string( NSStringEncoding *encoding = NULL ) const {
		OOString out = [[NSMutableString alloc] initWithContentsOfURL:get() usedEncoding:encoding error:&error];
		[*out release];
		return out;
	}
	oo_inline OOReference<NSData *> data( NSUInteger mask = 0 ) const {
		OOReference<NSData *> out = [[NSData alloc] initWithContentsOfURL:get() options:mask error:&error];
		[*out release];
		return out;
	}
	oo_inline id object() {
		return [NSKeyedUnarchiver unarchiveObjectWithData:data()];
	}
	oo_inline OORequest request() {
		return OORequest( get() );
	}
	oo_inline OOString post( const OOString &post ) {
		return request().post( post ).string();
	}
};

/**
 Placeholder for a file path, see OOResource, OODocument, OOTmpFile.
 */

class OOFile : public OOURL {
public:
	oo_inline OOFile( const OOString &path ) {
		setPath( path );
	}	
	oo_inline void setPath( const OOString &path ) {
		if ( *path )
			[set( [[NSURL alloc] initFileURLWithPath:path] ) release];
	}
	oo_inline OOString path() const {
		return [get() path];
	}
	oo_inline NSDictionary *attr() {
		return [[NSFileManager defaultManager] attributesOfItemAtPath:path() error:&error];
	}
	oo_inline unsigned long long size() {
		return [attr() fileSize];
	}

	oo_inline BOOL exists() const {
		return [[NSFileManager defaultManager] fileExistsAtPath:path()];
	}
	oo_inline BOOL save( const OOString &string, NSStringEncoding encoding = NSUTF8StringEncoding ) const {
		return save( [*string dataUsingEncoding:encoding allowLossyConversion:YES] );
	}
	oo_inline BOOL save( const NSData *data, BOOL atomically = NO ) const {
		return [data writeToFile:path() atomically:atomically];
	}
	oo_inline BOOL save( id object ) const {
		return save( [NSKeyedArchiver archivedDataWithRootObject:object] );
	}
};

/**
 Pathfinder for resources in applications.
 */

class OOResource : public OOFile {
public:
	oo_inline OOResource( const OOString &name ) : OOFile( nil ) {
		OOStringArray comps = name / @".";
		setPath( [[NSBundle mainBundle] pathForResource:**comps[0]
												 ofType:comps>1 ? **comps[1] : nil] );
	}
};

/**
 Pathfinder for documents for applications.
 */

class OODocument : public OOFile {
public:
	oo_inline OODocument( const OOString &name ) : OOFile( OOFormat( @"%@/Documents/%@", NSHomeDirectory(), *name ) ) {}
};

/**
 Pathfinder for temporary files for applications.
 */

class OOTmpFile : public OOFile {
public:
	oo_inline OOTmpFile( const OOString &name ) : OOFile( OOFormat( @"%@/tmp/%@", NSHomeDirectory(), *name ) ) {}
};

/**
 Internal class for subscripted acces to defaults.
 */

class OODefaultsSub : public OODictionarySub<OOString> {
	friend class OODefaults;
	oo_inline OODefaultsSub( const OODictionary<OOString> *root, id key ) : OODictionarySub<OOString>( root, key ) {
	}
	oo_inline virtual id set( id value ) {
		[[NSUserDefaults standardUserDefaults] setObject:value forKey:key];
		return OODictionarySub<OOString>::set( value );
	}
public:
	oo_inline OODefaultsSub &operator = ( OOString val ) { set( val ); return *this; }
	oo_inline OODefaultsSub &operator = ( CFStringRef val ) { set( (id)val ); return *this; }
	oo_inline OODefaultsSub &operator = ( NSString *val ) { set( val ); return *this; }
	oo_inline OODefaultsSub &operator = ( NSArray *val ) { set( val ); return *this; }
	oo_inline OODefaultsSub &operator = ( NSDictionary *val ) { set( val ); return *this; }
	oo_inline OODefaultsSub &operator = ( long long val ) { set( [[NSNumber numberWithLongLong:val] stringValue] ); return *this; }
	oo_inline OODefaultsSub &operator = ( double val ) { set( [[NSNumber numberWithDouble:val] stringValue] ); return *this; }
	oo_inline OODefaultsSub &operator = ( float val ) { set( [[NSNumber numberWithFloat:val] stringValue] ); return *this; }
	oo_inline OODefaultsSub &operator = ( int val ) { set( [[NSNumber numberWithInt:val] stringValue] ); return *this; }
	oo_inline OODefaultsSub &operator = ( BOOL val ) { set( [[NSNumber numberWithBool:val] stringValue] ); return *this; }
	oo_inline operator long long () { return [get() longLongValue]; }
	oo_inline operator double () { return [get() doubleValue]; }
	oo_inline operator float () { return [get() floatValue]; }
	oo_inline operator int () { return [get() intValue]; }
	oo_inline operator NSDictionary * () { return get(); }
	oo_inline operator NSArray * () { return get(); }
	oo_inline OOString operator ~ () {
		[[NSUserDefaults standardUserDefaults] removeObjectForKey:key];
		return OODictionarySub<OOString>::operator ~ ();
	}
};

/**
 Binds defaults to an OODictionary.
 */

class OODefaults : OODictionary<OOString> {
public:
	oo_inline OODefaults() {
		[set( [[[NSUserDefaults standardUserDefaults] dictionaryRepresentation] mutableCopy] ) release];
	}
	oo_inline OODefaultsSub operator[] ( id key ) const {
		return OODefaultsSub( this, key );
	}
	oo_inline OODefaultsSub operator [] ( const CFStringRef sub ) const {
		return (*this)[(id)sub];
	}	
	oo_inline void sync() {
		[[NSUserDefaults standardUserDefaults] synchronize];
	}
	oo_inline ~OODefaults()  {
		sync();
	}
};

/**
 Binds application's Info.plist to an OODictionary.
 */

class OOPlist : public OODictionary<OOString> {
public:
	OOPlist() {
		set( [[NSBundle mainBundle] infoDictionary] );
	}
};

/**
 Shortcut for creating alerts.
 */

class OOAlert {
public:
	OOAlert( OOString msg, id del = nil, 
			OOString cancel = @"OK", OOString b1 = nil, OOString b2 = nil ) {
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
		[[[[UIAlertView alloc] initWithTitle:*OOPlist()[@"CFBundleDisplayName"] message:msg delegate:del
						   cancelButtonTitle:cancel otherButtonTitles:*b1, *b2, nil] autorelease] show];
#else
		OOWarn( @"Alert: %@", *msg );
#endif		
	}
};	

/*=================================================================================*/
/*================================ Leftovers ======================================*/

/**
 A wrapper for a number using NSNumber so it can be put in an NSArray or NSDictionary
 */

class OONumber : public OOReference<NSNumber *> {
public:
	OONumber( double d ) {
		[set( [[NSNumber alloc] initWithDouble:d] ) release];
	}
	OONumber( NSNumber *val ) {
		set( val );
	}
	
	operator NSNumber * () const { return get(); } 
	operator double () const { return [get() doubleValue]; } 
	double operator * () const { return [get() doubleValue]; }
	
	OONumber &operator = ( const OONumber &val ) { set( val.get() ); return *this; }
	OONumber &operator += ( double val ) { return *this = *this + val; }
	OONumber &operator -= ( double val ) { return *this = *this - val; }
	OONumber &operator *= ( double val ) { return *this = *this * val; }
	OONumber &operator /= ( double val ) { return *this = *this / val; }
	OONumber operator + ( double val ) const { return **this + val; }
	OONumber operator - ( double val ) const { return **this - val; }
	OONumber operator * ( double val ) const { return **this * val; }
	OONumber operator / ( double val ) const { return **this / val; }
};

/**
 A fledgling wrapper for NSScanner which didn't really pan out...
 */

class OOScan {
	OOReference<NSScanner *> scan;
public:
	oo_inline OOScan( const OOString &input ) {
		[*(scan = [[NSScanner alloc] initWithString:input]) release];
	}
	
	oo_inline OOString operator & ( NSString *str ) {
		NSString *out = nil;
		[*scan scanString:str intoString:&out];
		return out;
	}
	oo_inline OOString operator | ( NSString *str ) {
		NSString *out = nil;
		[*scan scanUpToString:str intoString:&out];
		return out;
	}
	oo_inline OOScan &operator >> ( double &d ) {
		[*scan scanDouble:&d];
		return *this;
	}
	oo_inline OOScan &operator >> ( float &f ) {
		[*scan scanFloat:&f];
		return *this;
	}
	oo_inline OOScan &operator >> ( int &i ) {
		[*scan scanInt:&i];
		return *this;
	}
};


