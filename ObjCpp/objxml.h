/*
 *  objxml.h - OODictionary representation of XML
 *  ========
 *
 *  Created by John Holdsworth on 01/04/2009.
 *  Copyright 2009 John Holdsworth.
 *
 *  $Id: //depot/2.2/ObjCpp/objxml.h#1 $
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

#import <libxml/tree.h>
#import <libxml/xmlwriter.h>

#define OONodeArray OOArray<OONode>
#define OONodes OONodeArray 

static NSString *kOOChildren = @".children", *kOOTagName = @"@tagName", *kOOTagPrefix = @"@tagPrefix", *kOONodeText = @".nodeText";

/*=================================================================================*/
/*============ Parse NSData XML into OODictionary representation ==================*/

enum OOXMLParserOpts {
	OOXMLDefaultParser = 0x0,
	OOXMLPreserveWhitespace = 0x1,
	OOXMLPreserveCData = 0x2,
	OOXMLRecursive = 0x4,
	// not implemented...
	OOXMLRecursiveAtAnyLevel = 0x4,
	OOXMLNamespaceSelect = 0x10,
	OOXMLNamespaceSelectAtAnyLevel = 0x20
};

enum OOXMLWriterOpts {
	OOXMLPrettyPrint = 0x1,
	OOXMLDefaultWriter = OOXMLPrettyPrint
};

class OONodeSub;

/**
 Subclass of OODictionary<OOString> to manipulate XML documents. On parsing as XML document
 one OONode is created for each element in the document with the attribute values stored
 as a NSString under a key: "@attributeName". Children of an element, be they pure text
 or sub elements are accumulated in an NSMutableArray under the key @".children" as well
 as in an array with the elements tagName (element name) as the key. This allows simple
 xpath expressions such as document[@"root/child"] to be evaluated by breaking up the
 path into the equivalent of document[@"root"][0][@"child"][0]. To gain efficient
 access to the text inside an element it is also stores under the key @".nodeText"
 in it's parent element.
 */

class OONode : public OODictionary<OOString> {
public:
	oo_inline OONode( id node ) {
		set( node );
	}
	oo_inline OONode( const OONode &node ) { 
		*this = node;
	}
	oo_inline OONode() {
		[alloc() setObject:(id)kCFNull forKey:kOOTagName];
	}
	oo_inline OONode( NSString *tagName ) {
		[alloc() setObject:tagName forKey:kOOTagName];
	}
	oo_inline OONode( const OOString &tagName, const OOString &nodeText = nil ) {
		[alloc() setObject:tagName forKey:kOOTagName];
		if ( nodeText ) {
			[get() setObject:nodeText forKey:kOONodeText];
			OODictionary<OOArray<OOString> > node = get();
			NSMutableArray *children = node[kOOChildren].alloc( [NSMutableArray class] );
			[children addObject:nodeText];
		}
	}
	oo_inline OONode( NSData *xml, OOXMLParserOpts flags = OOXMLDefaultParser ) {
		parseXML( xml, flags );
	}
	
	oo_inline OONode &operator = ( const OONode &node ) { set( node.get() ); return *this; }
	oo_inline OONode &operator = ( NSData *val ) { return parseXML( val ); }

	OONode &parseXML( NSData *xml, OOXMLParserOpts flags = OOXMLDefaultParser );
	OOReference<NSData *> writeXML( OOXMLWriterOpts flags = OOXMLDefaultWriter ) const;

	oo_inline operator OOReference<NSData *> () const {
		return writeXML();
	}
	
	oo_inline OONode &operator += ( const OONode &val ) {
		OODictionary<OONodeArray > node = get();
		NSMutableArray *children = node[kOOChildren].alloc( [NSMutableArray class] );
		[children addObject:val.get()];
		NSString *tagName = [val objectForKey:kOOTagName];
		if ( tagName && tagName != (id)kCFNull ) {
			NSMutableArray *siblings = node[tagName].alloc( [NSMutableArray class] );
			[siblings addObject:val.get()];
		}
		return *this;
	}
	
	oo_inline OONodeSub operator [] ( id sub ) const;
	oo_inline OONodeSub operator [] ( const char *sub ) const;
	
	oo_inline OONodeArray children() const {
		return [get() objectForKey:kOOChildren];
	}
	oo_inline OONode child( int which = 0 ) const {
		return children()[ which ];
	}
	oo_inline OOString text( int which = 0 ) const {
		return (id)child( which ).get();
	}
	
	oo_inline OOArraySub<OONode> operator [] ( int sub ) const;
};

/**
 Internal class repesenting selecting of a node from an xpath selection.
 */

class OONodeArraySub : public OOArraySub<OOString> {
	friend class OONodeSub;

	OONodeArraySub( OODictionarySub<NSMutableArray *> *ref, int sub ) : OOArraySub<OOString>( ref,  sub ) { }

	OOArraySub<NSMutableDictionary *> *nodeReference( BOOL refer = YES ) {
		if ( refer && this->dref->aref->references )
			this->dref->aref->references++;
		return this->dref->aref;
	}

protected:
	oo_inline virtual id set( id val ) const {
		OOArraySub<OOString>::set( val );
		if ( ![kOOChildren isEqualToString:dref->key] ) {
			NSMutableDictionary *parentNode = root ? (NSMutableDictionary *)root->get() : dref->parent( YES );
			OODictionary<OONodeArray> node( parentNode );
			OONodeArray children = node[kOOChildren].alloc( [NSMutableArray class] );
			children += OONode( val );
			if ( ![val objectForKey:kOOTagName] )
				[val setObject:dref->key forKey:kOOTagName];
		}
		return val;
	}

public:
	oo_inline OONode node() {
		return nodeReference( NO )->get();	
	}
	oo_inline operator OONode () {
		return node();
	}
	
	oo_inline OONodeSub operator [] ( id sub );
	oo_inline OONodeSub operator [] ( const char *sub );
};

/**
 Internal class repesenting selection of node from XPath expression. Normally this
 is the OOString value of the first node selected by the xpath expression but can
 be cast to the first node itself or an array of all qualifying nodes.
 */

class OONodeSub : public OODictionarySub<OOString> {
	friend class OONodeArraySub;
	friend class OONode;	
	
	void supportXPath() {
		unichar char0 = [key length]>0 ? [key characterAtIndex:0] : 0;
		if ( char0 != '@' && char0 != '.' ) {
			NSArray *path = [this->key componentsSeparatedByString:@"/"];
			int pmax = [path count]-1, firstCharOfLast = [[path objectAtIndex:pmax] length] ? 
			[[path objectAtIndex:pmax] characterAtIndex:0] : 0;

			if ( firstCharOfLast != '@' && firstCharOfLast != '.' ) {
				[(NSMutableArray *)(path = [[path mutableCopy] autorelease]) addObject:kOONodeText];
				pmax++;
			}
			
			if ( pmax > 0 ) {
				OODictionarySub<NSMutableArray *> *exp = this->root ? 
				new OODictionarySub<NSMutableArray *>( this->root, [path objectAtIndex:0] ) :
				new OODictionarySub<NSMutableArray *>( this->aref, [path objectAtIndex:0] );
				exp->references = 1;
				
				OONodeArraySub *arr;
				for ( int i=1 ; i<=pmax ; i++ ) {
					int idx = 0;
					NSString *p = [path objectAtIndex:i];
					if ( [p length] && isdigit( [p characterAtIndex:0] ) ) {
						idx = [p intValue];
						i++;
					}
					arr = new OONodeArraySub( exp, idx );
					arr->references = 1;
					if ( i == pmax )
						break;
					if ( [@"*" isEqualToString:p = [path objectAtIndex:i]] )
						p = kOOChildren;
					exp = new OODictionarySub<NSMutableArray *>( (OOArraySub<NSMutableDictionary *> *)arr, p );
					exp->references = 1;
				}
				
				this->key = [path objectAtIndex:pmax];
				this->root = NULL;
				this->aref = (OOArraySub<NSMutableDictionary *> *)arr;
				this->dref = NULL;
			}
		}
	}

	oo_inline OONodeSub( const OODictionary<OOString> *ref, id sub ) : OODictionarySub<OOString>( ref, sub ) {
		supportXPath();
	}
	oo_inline OONodeSub( OOArraySub<NSMutableDictionary *> *ref, id sub ) : OODictionarySub<OOString>( ref, sub ) {
		supportXPath();
	}
	oo_inline OONodeSub( OODictionarySub<NSMutableDictionary *> *ref, id sub ) : OODictionarySub<OOString>( ref, sub ) {
		supportXPath();
	}

protected:
	oo_inline virtual id set( id val ) const {
		OODictionarySub<OOString>::set( val );
		if ( [kOONodeText isEqualToString:this->key] ) {
			OODictionary<OOArray<OOString> > node = this->parent( YES );
			NSMutableArray *children = node[kOOChildren].alloc( [NSMutableArray class] );
			[children addObject:val];
		}
		return val;
	}
	
public:
	// assign and assign by mutable copy 
	oo_inline OONodeSub &operator = ( OOString val ) { set( val ); return *this; }
	oo_inline OONodeSub &operator = ( const char *val ) { *this = OOString( val ); return *this; }
	oo_inline OONodeSub &operator = ( const OOArraySub<OOString> &val ) { set( val.get() ); return *this; }
	oo_inline OONodeSub &operator = ( const OODictionarySub<OOString> &val ) { set( val.get() ); return *this; }

	oo_inline OONodeSub &operator = ( const OONode &val ) { this->aref->set( val.get() ); return *this; }
	oo_inline OONodeSub &operator += ( const OONode &val ) { OONode( this->parent(YES) ) += val; return *this; }

	oo_inline OONodeSub operator [] ( id sub ) const {
		if ( this->aref->references )
			this->aref->references++;
		return OONodeSub( this->aref, sub );
	}
	oo_inline OONodeSub operator [] ( const char *sub ) const {
		return (*this)[[NSString stringWithUTF8String:sub]];
	}	
	
	oo_inline OONodeArraySub operator [] ( int sub ) const {
		if ( this->aref->dref->references )
			this->aref->dref->references++;
		OONodeArraySub *node = new OONodeArraySub( this->aref->dref, sub );
		node->references = 1;
		OONodeSub *children = new OONodeSub( (OOArraySub<NSMutableDictionary *> *)node, kOOChildren );
		children->references = 1;
		return OONodeArraySub( (OODictionarySub<NSMutableArray*> *)children, 0 );
	}

	oo_inline operator OONodeArray () const {
		return nodes();
	}
	inline operator OONode () const {
		return node();
	}
	
	oo_inline OONodeArray nodes() const {
		return this->aref ? this->aref->parent( NO ) : nil;
	}
	inline OONode node( int which = 0 ) const {
		return nodes()[which];
	}

	oo_inline OONodeArray children() const {
		return [this->parent( NO ) objectForKey:kOOChildren];
	}
	inline OONode child( int which = 0 ) const {
		return children()[which];
	}
	
	inline OOStringDictionaryArray dictionaries() {
		OOStringDictionaryArray out;
		OONodeArray in = nodes();
		for ( NSMutableDictionary *n in *in ) {
			OOStringDictionary values;
			OONode node = n;
			for( NSString *key in [*node allKeys] ) {
				unichar c0 = [key characterAtIndex:0];
				if ( c0 == '.' || c0 == '@' )
					continue;
				values[key] = node[key];
			}
			out += values;
		}
		return out;
	}
	oo_inline operator OOStringDictionaryArray () {
		return dictionaries();
	}	
};

inline OONodeSub OONode::operator [] ( id sub ) const {
	return OONodeSub( this, sub );
}

inline OOArraySub<OONode> OONode::operator [] ( int sub ) const {
	OODictionarySub<OONode> *step1 = new OODictionarySub<OONode>( (OODictionary<OONode> *)this, kOOChildren );
	step1->references = 1;
	return (*step1)[sub];
}

inline OONodeSub OONode::operator [] ( const char *sub ) const {
	return (*this)[[NSString stringWithUTF8String:sub]];
}

inline OONodeSub OONodeArraySub::operator [] ( id sub ) {
	return OONodeSub( nodeReference(), sub );
}

inline OONodeSub OONodeArraySub::operator [] ( const char *sub ) {
	return OONodeSub( nodeReference(), [NSString stringWithUTF8String:sub] );
}

/*=================================================================================*/
/*============ Parse XML NSData into OODictionary representation ==================*/

static xmlSAXHandler ooSaxHandlers;

/**
 SAX Parses an XML document into an OODictionary based represntation. See OONode class
 for description of structure generated.
 */

class OOXMLSaxParser {
	OODictionary<NSString *> tagCache;
	xmlParserCtxtPtr context;
public:
	OOXMLParserOpts flags;
	OOArray<id> children;
	OONodeArray stack;
	OONode index;

	OOXMLSaxParser( OOXMLParserOpts flags = OOXMLDefaultParser );
	
	oo_inline NSMutableString *unique( const char *value ) {
		NSString *tagString = [[NSString alloc] initWithUTF8String:value],
					*unique = tagCache[tagString];
		if ( !unique )
			tagCache[tagString] = unique = tagString;
		[tagString release];
		return (NSMutableString *)unique;
	}

	oo_inline void addNode( id node, const char *utf8String = NULL, int len = 0 ) {
		OONode parent = stack[-1];
		if ( !children )
			children = parent[kOOChildren].alloc( [NSMutableArray class] );

		if ( utf8String ) {
			static OOPattern nonWhiteSpace = OOString( @"\\S" );
			OOArray<OOString> textChildren = children;
			regmatch_t match[] = {{0, len}};
			OOString text = node;

			if ( children>0 && [*children[-1] isKindOfClass:[NSMutableString class]] )
				*textChildren[-1] += node;
			else if ( flags & OOXMLPreserveWhitespace || nonWhiteSpace.exec( utf8String, 0, match, REG_STARTEND ) ) {
				textChildren += text;
				if ( ![*parent objectForKey:kOONodeText] )
					[*parent setObject:node forKey:kOONodeText];
			}
		}
		else
			children += node;
	}

	oo_inline int parse( NSData *chunk ) {
		if ( !context ) {
			OONode root;
			if ( flags & OOXMLRecursive )
				root["/"] = index = OONode();
			stack = OONodeArray( root, nil );
			context = xmlCreatePushParserCtxt( &ooSaxHandlers, this, NULL, 0, NULL);
			children = 0;
		}
		
		return xmlParseChunk(context, (const char *)[chunk bytes], [chunk length], 0);
	}

	oo_inline OONode rootNodeForXMLData( NSData *xml = nil ) {
		if ( xml )
			parse( xml );
		xmlParseChunk(context, NULL, 0, 1);
		xmlFreeParserCtxt(context);
		context = NULL;
		return --stack;
	}
};

static void objcppStartElement(void *ctx, const xmlChar *localname, const xmlChar *prefix, const xmlChar *URI, 
							   int nb_namespaces, const xmlChar **namespaces, 
							   int nb_attributes, int nb_defaulted, const xmlChar **attributes) {
	OOXMLSaxParser &sax = *(OOXMLSaxParser *)ctx;
	OOString tagName = sax.unique( (const char *)localname );
	OONode element = OONode( tagName );
	char name[10000];

	if ( prefix )
		[element setObject:sax.unique( (const char *)prefix ) forKey:kOOTagPrefix];

	for ( int ns=0 ; ns < nb_namespaces ; ns++ ) {
		struct _ns { const char *prefix, *nsURI; } *nptr = 
		(struct _ns *)(namespaces + ns*sizeof *nptr/sizeof nptr->prefix);

		snprintf( name, sizeof name-1, nptr->prefix ? "@xmlns:%s" : "@xmlns", nptr->prefix );
		[element setObject:sax.unique( nptr->nsURI ) forKey:sax.unique( name )];
	}

	for ( int attr_no=0 ; attr_no < nb_attributes ; attr_no++ ) {
		struct _attrs { const char *localName, *prefix, *uri, *value, *end; } *aptr = 
		(struct _attrs *)(attributes + attr_no*sizeof *aptr/sizeof aptr->localName);
		int vlen = aptr->end-aptr->value;

		OOString value( aptr->value, vlen );
		// attribute value fix required due to libxml2 bug...
		if ( strnstr( aptr->value, "&#38;", vlen ) != NULL )
			value = [value stringByReplacingOccurrencesOfString:@"&#38;" withString:@"&"];

		snprintf( name, sizeof name-1, "@%s", aptr->localName );
		[element setObject:sax.unique( value ) forKey:sax.unique( name )];
	}

	sax.stack += element;
	sax.children = 0;
};

static void	objcppEndElement(void *ctx, const xmlChar *localname, const xmlChar *prefix, const xmlChar *URI) {    
	OOXMLSaxParser &sax = *(OOXMLSaxParser *)ctx;
	OONode element = sax.stack--;
	OONode parent = sax.stack[-1];
	parent += element;
	if ( sax.flags & OOXMLRecursive )
		sax.index += element;
	sax.children = 0;
}

static void	objcppCharacters(void *ctx, const xmlChar *ch, int len) {
	OOXMLSaxParser &sax = *(OOXMLSaxParser *)ctx;
	sax.addNode( OOString( (const char *)ch, len ), (const char *)ch, len );
}

static void objcppCData(void *ctx, const xmlChar *value, int len) {
	OOXMLSaxParser &sax = *(OOXMLSaxParser *)ctx;
	NSData *data = [[NSData alloc] initWithBytes:value length:len];
	sax.addNode( data );
	[data release];
}

static void objcppSAXError(void *ctx, const char *msg, ...) {
	va_list argp; va_start(argp, msg);
	NSLogv( "OOXMLSaxParser Error - "+OOString( msg ), argp );
	va_end( argp );
}

inline OOXMLSaxParser::OOXMLSaxParser( OOXMLParserOpts flags ) {
	this->flags = flags;

	if ( !ooSaxHandlers.initialized ) {
		ooSaxHandlers.startElementNs = objcppStartElement;
		ooSaxHandlers.endElementNs = objcppEndElement;
		ooSaxHandlers.characters = objcppCharacters;
		ooSaxHandlers.initialized = XML_SAX2_MAGIC;
		ooSaxHandlers.error = objcppSAXError;
	}
	
	ooSaxHandlers.cdataBlock = this->flags & OOXMLPreserveCData ? objcppCData : NULL;
	context = NULL;
}

inline OONode &OONode::parseXML( NSData *xml, OOXMLParserOpts flags ) {
	return *this = OOXMLSaxParser( flags ).rootNodeForXMLData( xml );
}

/*=================================================================================*/
/*======================== Convert Dictionary back to NSData XML ==================*/

/**
 Class to convert OODictionary representation of XML into an NSData structure to be written to the net.
 */

class OOXMLWriter {
public:
	OOXMLWriterOpts flags;
	xmlTextWriterPtr writer;
    char name[1000], value[10000];
	int level;

	oo_inline void indent() {
		static char spaces[] = "                                                                ";
		checkrc( "xmlTextWriterWriteFormatString", xmlTextWriterWriteFormatString( writer, "\n%.*s", level*2, spaces ) );
	}
	
	oo_inline void checkrc( const char *what, int rc ) {
		if ( rc < 0 )
			OOWarn( @"Error code returned from %s(): %d", what, rc );
	}
	
	void traverse( OONode node ) {
		NSString *tagName = [*node objectForKey:kOOTagName];

		if ( tagName && tagName != (id)kCFNull ) {
			if ( flags & OOXMLPrettyPrint && level )
				indent();
			level++;

			NSString *tagPrefix = [*node objectForKey:kOOTagPrefix];
			if ( tagPrefix ) {
				[tagPrefix getCString:name maxLength:sizeof name-1 encoding:NSUTF8StringEncoding];
				strcat( name, ":" );
			}
			else
				name[0] = '\000';
			
			[tagName getCString:name+strlen(name) maxLength:sizeof name-1-strlen(name) encoding:NSUTF8StringEncoding];
			checkrc( "xmlTextWriterStartElement", xmlTextWriterStartElement( writer, (xmlChar *)name ) );

			for ( NSString *attr in [*node allKeys] ) {
				if ( attr == kOOTagName || attr == kOOTagPrefix || [attr length] == 0 || [attr characterAtIndex:0] != '@' )
					continue;
				[attr getCString:name maxLength:sizeof name-1 encoding:NSUTF8StringEncoding];
				[[*node objectForKey:attr] getCString:value maxLength:sizeof value-1 encoding:NSUTF8StringEncoding];
				checkrc( "xmlTextWriterWriteAttribute", xmlTextWriterWriteAttribute( writer, (xmlChar *)name+1, (xmlChar *)value ) );
			}			
		}

		BOOL hadChildElements = NO;
		OONodeArray children = node.children();
		for ( id child in *children )
			if ( [child isKindOfClass:[NSString class]] )
				checkrc( "xmlTextWriterWriteString", xmlTextWriterWriteString( writer, (xmlChar *)[child UTF8String] ) );
			else if ( [child isKindOfClass:[NSData class]] )
				checkrc( "xmlTextWriterWriteFormatCDATA", xmlTextWriterWriteFormatCDATA( writer, "%.*s", [child length], (char *)[child bytes] ) );
			else {
				traverse( child );
				hadChildElements = YES;
			}

		if ( tagName != (id)kCFNull ) {
			level--;
			if ( flags & OOXMLPrettyPrint && hadChildElements )
				indent();
			checkrc( "xmlTextWriterEndElement", xmlTextWriterEndElement( writer ) );
		}
	}

	oo_inline OOXMLWriter( OOXMLWriterOpts flags = OOXMLDefaultWriter ) {
		this->flags = flags;
		level = 0;
	}

	oo_inline OOReference<NSData *> dataForNode( const OONode &node ) {
		xmlBufferPtr buf = xmlBufferCreate();
		writer = xmlNewTextWriterMemory(buf, 0);

		NSString *encoding = *node[@"@encoding"];
		checkrc( "xmlTextWriterStartDocument", xmlTextWriterStartDocument(writer, NULL, encoding ? [encoding UTF8String] : "UTF-8", NULL) );

		traverse( node );

		xmlTextWriterEndDocument(writer);
		xmlFreeTextWriter(writer);

		OOReference<NSData *> data = [[NSData alloc] initWithBytes:buf->content length:strlen((const char *)buf->content)];
		xmlBufferFree(buf);
		[*data release];
		return data;
	}
};

inline OOReference<NSData *> OONode::writeXML( OOXMLWriterOpts flags ) const {
	return OOXMLWriter( flags ).dataForNode( *this );
}
