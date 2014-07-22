#include "PList.h"

#include <QTextStream>
#include <iostream>
#include <typeinfo>
#include <algorithm>

const char EOL_CHAR = 0x0a;

void PList::TextStream::putChar( QChar c )
{
	mOverflow.push_back( c );
	if( c == EOL_CHAR )
		mCurrentLine--;
}

QChar PList::TextStream::readChar()
{
	if( mOverflow.empty() ) {
		QChar c;
		(*mStream) >> c;
		if( c == EOL_CHAR ) {
			mCurrentLine++;
		}
		return c;
	}
	else {
		QChar c = mOverflow.front();
		mOverflow.pop_front();
		if( c == EOL_CHAR )
			mCurrentLine++;
		return c;
	}
}

QString PList::TextStream::readLine()
{
	QChar c;
	QString result;
	do {
		c = readChar();
		if( c != EOL_CHAR )
			result.append( c );
	} while( c != EOL_CHAR && ( ! atEnd() ) );
	return result;
}

QString PList::TextStream::readString()
{
	QChar c;
	QString result;
	do {
		c = readChar();
		if( ! c.isSpace() )
			result.append( c );
	} while( ! c.isSpace() );
	return result;
}

bool PList::TextStream::readComment( QString *result )
{
	// if the first char is a * then it's c-style
	QChar firstChar = readChar();
	if( firstChar != '/' ) {
		if( result )
			result->clear();
		return false;
	}
	QChar secondChar = readChar();
	if( secondChar == '*' ) { // C-Style
		bool done = false;
		if( result ) result->append( firstChar );
		if( result ) result->append( secondChar );
		bool gotAsterisk = false;
		while( ! done ) {
			QChar c = readChar();
			if( c == '*' ) {
				if( result ) result->append( c );
				gotAsterisk = true;
			}
			else if( c == '/' && gotAsterisk ) {
				if( result ) result->append( c );
				done = true;
			}
			else {
				gotAsterisk = false;
				if( result ) result->append( c );
			}
		}

		return true;
	}
	else if( secondChar == '/' ) { // C++-style
		if( result ) *result = readLine();
		else readLine();
		return true;
	}
	else {
		putChar( firstChar );
		putChar( secondChar );
		if( result )
			result->clear();
		return false;
	}
}

bool PList::TextStream::atEnd()
{
	return mStream->atEnd();
}

void PList::TextStream::skipUntilChar( QChar searchChar )
{
	QChar c;
	do {
		c = readChar();
	} while( ! mStream->atEnd() && ( c != searchChar ) );

	putChar( c );
}

void PList::TextStream::skipUntilChars( QString searchChars, QString *comment )
{
	if( comment )
		comment->clear();

	QChar c;
	do {
		c = readChar();
		if( comment && ( c == '/' ) ) { // could be the start of a comment we need to capture
			putChar( c );
			readComment( comment );
			c = readChar();
		}
	} while( ! mStream->atEnd() && ( ! searchChars.contains( c ) ) );

	putChar( c );
}

void PList::TextStream::skipWhiteSpace( QString *comment )
{
	bool done = false;
	while( ! done ) {
		QChar c;
		do {
			c = readChar();
		} while( ! mStream->atEnd() && ( c.isSpace() ) );
		if( c == '/' ) { // comment
			putChar( c );
			if( ! readComment( comment ) )
				done = true;
		}
		else {
			putChar( c );
			done = true;
		}
	}
}

PList::PList( QFile &file )
{
	parse( file );
}

PList::PList( const QString &s )
{
	parse( s );
}

void PList::parse( QFile &file )
{
	if( ! file.open( QFile::ReadOnly | QFile::Text ) ) {
		throw ParseFailed();
	}

	QTextStream *qs = new QTextStream( &file );
	qs->setAutoDetectUnicode( false );
	qs->setCodec( "UTF-8" );
	PList::TextStream s = PList::TextStream( QSharedPointer<QTextStream>( qs ) );

	s.skipWhiteSpace();
	mContents = parseItem( s, "" );
std::cout << "Found " << s.mCurrentLine << " lines" << std::endl;
}

void PList::parse( const QString &s )
{
	QString localCopy = s;
	QTextStream *qs = new QTextStream( &localCopy );
	qs->setAutoDetectUnicode( false );
	qs->setCodec( "UTF-8" );
	PList::TextStream ts = PList::TextStream( QSharedPointer<QTextStream>( qs ) );

	ts.skipWhiteSpace();
	mContents = parseItem( ts, "" );
std::cout << "Found " << ts.mCurrentLine << " lines" << std::endl;
}

bool PList::insertBeforeHelper( QSharedPointer<Item> parent, QSharedPointer<Item> before, QSharedPointer<Item> value )
{
	if( typeid(*parent) == typeid(Dictionary) ) {
		QSharedPointer<Dictionary> parentDict = parent.dynamicCast<Dictionary>();
		QList<QSharedPointer<Item> >::iterator itIt = std::find( parentDict->mItems.begin(), parentDict->mItems.end(), before );
		if( itIt != parentDict->mItems.end() ) {
			parentDict->mItems.insert( itIt, value );
			return true;
		}
		// walk children
		for( itIt = parentDict->mItems.begin(); itIt != parentDict->mItems.end(); ++itIt ) {
			if( insertBeforeHelper( *itIt, before, value ) )
				return true;
		}
		return false;
	}
	else if( typeid(*parent) == typeid(Array) ) {
		QSharedPointer<Array> parentArray = parent.dynamicCast<Array>();
		QList<QSharedPointer<Item> >::iterator itIt = std::find( parentArray->mItems.begin(), parentArray->mItems.end(), before );
		if( itIt != parentArray->mItems.end() ) {
			parentArray->mItems.insert( itIt, value );
			return true;
		}
		// walk children
		for( itIt = parentArray->mItems.begin(); itIt != parentArray->mItems.end(); ++itIt ) {
			if( insertBeforeHelper( *itIt, before, value ) )
				return true;
		}
		return false;
	}
	else
		return false;
}

void PList::insertBefore( QSharedPointer<Item> before, QSharedPointer<Item> value )
{
	if( insertBeforeHelper( mContents, before, value ) )
		return;
	else {
		QSharedPointer<Dictionary> contentsDict = mContents.dynamicCast<Dictionary>();
		contentsDict->mItems.push_back( value );
	}
}

bool PList::insertAfterHelper( QSharedPointer<Item> parent, QSharedPointer<Item> after, QSharedPointer<Item> value )
{
	if( typeid(*parent) == typeid(Dictionary) ) {
		QSharedPointer<Dictionary> parentDict = parent.dynamicCast<Dictionary>();
		QList<QSharedPointer<Item> >::iterator itIt = std::find( parentDict->mItems.begin(), parentDict->mItems.end(), after );
		if( itIt != parentDict->mItems.end() ) {
			++itIt;
			parentDict->mItems.insert( itIt, value );
			return true;
		}
		// walk children
		for( itIt = parentDict->mItems.begin(); itIt != parentDict->mItems.end(); ++itIt ) {
			if( insertAfterHelper( *itIt, after, value ) )
				return true;
		}
		return false;
	}
	else if( typeid(*parent) == typeid(Array) ) {
		QSharedPointer<Array> parentArray = parent.dynamicCast<Array>();
		QList<QSharedPointer<Item> >::iterator itIt = std::find( parentArray->mItems.begin(), parentArray->mItems.end(), after );
		if( itIt != parentArray->mItems.end() ) {
			++itIt;
			parentArray->mItems.insert( itIt, value );
			return true;
		}
		// walk children
		for( itIt = parentArray->mItems.begin(); itIt != parentArray->mItems.end(); ++itIt ) {
			if( insertAfterHelper( *itIt, after, value ) )
				return true;
		}
		return false;
	}
	else
		return false;
}

void PList::insertAfter( QSharedPointer<Item> after, QSharedPointer<Item> value )
{
	if( insertAfterHelper( mContents, after, value ) )
		return;
	else {
		QSharedPointer<Dictionary> contentsDict = mContents.dynamicCast<Dictionary>();
		contentsDict->mItems.push_back( value );
	}
}

bool PList::needsQuotes( const QString &str )
{
	if( str.contains( '<' ) || str.contains( '>') ||
		str.contains( '(' ) || str.contains( ')') ||
		str.contains( '{' ) || str.contains( '}') ||
		str.contains( '[' ) || str.contains( ']') ||
		str.contains( '=' ) || str.contains( '"' ) ||
		str.contains( ' ' ) || str.contains( "@" ) ||
		str.contains( '+' ) || str.contains( "-" ) ||
		str.isEmpty()
	 )
		return true;

	return false;
}

QChar PList::parseUntilChar( TextStream &s, QChar c )
{
	s.skipUntilChar( c );
	return s.readChar();
}

QChar PList::parseNonWhitespaceChar( TextStream &s, QString *suffixComment )
{
	s.skipWhiteSpace( suffixComment );
	return s.readChar();
}

void PList::parseComment( TextStream &s )
{
	// if the first char is a * then it's c-style
	QChar firstChar = s.readChar();
	if( firstChar == '*' ) { // C-Style
		bool done = false;
		while( ! done ) {
			QChar c = parseUntilChar( s, '*' );
			if( c == '/' )
				done = true;
		}
	}
	else if( firstChar == '/' ) { // C++-style
		s.readLine();
	}
	else {
		throw ParseFailed();
	}
}

bool PList::parseValue( TextStream &s, QString *result, QString *suffixComment )
{
	bool done = false, seenNonWhiteSpace = false;
	do {
		s.skipWhiteSpace( ( seenNonWhiteSpace ) ? suffixComment : 0 );
		seenNonWhiteSpace = true;
		QChar c = s.readChar();

		if( c == ';' || c == ',' ) {
			done = true;
			s.putChar( c );
		}
		else
			*result = result->append( c );
	} while( ( ! done ) && ( ! s.atEnd() ) );

	return true;
}

bool PList::parseQuotedString( TextStream &s, QString *result )
{
	bool done = false, escaping = false;
	do {
		QChar c = s.readChar();
		if( c == '\\' ) {
			if( escaping ) {
				*result = result->append( '\\' );
				escaping = false;
			}
			else
				escaping = true;
		}
		else if( c == '"' && escaping ) {
			*result = result->append( '"' );
			escaping = false;
		}
		else if( c == '"' && ( ! escaping ) )
			done = true;
		else if( escaping ) {
			*result = result->append( '\\' );
			*result = result->append( c );
		}
		else
			*result = result->append( c );
	} while( ! done );

	return true;
}

bool PList::parseTag( TextStream &s, QString *result, bool *resultQuoted, QString *resultSuffixComment )
{
	if( resultSuffixComment )
		resultSuffixComment->clear();

	s.skipWhiteSpace();
	QChar firstChar = s.readChar();

	if( firstChar == '}' || firstChar == ')' )
		return false;
	if( firstChar == '"' ) {
		result->clear();
		if( ! parseQuotedString( s, result ) )
			throw ParseFailed();
		*resultQuoted = true;
	}
	else {
		*result = s.readString();
		*result = result->prepend( firstChar );
		*resultQuoted = false;
	}

	// eat up the equals and the whitespace
	QChar equals = parseNonWhitespaceChar( s, resultSuffixComment );
	if( equals != '=' )
		throw ParseFailed();

	return true;
}

QSharedPointer<PList::Item> PList::parseItem( TextStream &s, const QString &tag )
{
	QChar firstChar = parseNonWhitespaceChar( s );
	if( firstChar == '{' ) {
		return parseDictionary( s, tag );
	}
	else if( firstChar == '(' ) {
		return parseArray( s, tag );
	}
	else if( firstChar == ')' ) { // end of a list; return null
		return QSharedPointer<PList::Item>();
	}
	else if( firstChar == '"' ) { // string, part of a value
		QString word;
		if( ! parseQuotedString( s, &word ) )
			throw ParseFailed();
		QSharedPointer<Item> result( new ValueString( tag, word, true ) );
		return result;
	}
	else {// a non-quoted string?
		QString word = firstChar, suffixComment;
		if( ! parseValue( s, &word, &suffixComment ) )
			throw ParseFailed();
		QSharedPointer<Item> result( new ValueString( tag, word, false ) );
		result->setValueSuffixComment( suffixComment );
		return result;
	}
}

QSharedPointer<PList::Dictionary> PList::parseDictionary( TextStream &s, const QString &tag )
{
	QSharedPointer<PList::Dictionary> result( new PList::Dictionary( tag ) );

	QString itemTag, tagSuffixComment;
	bool itemTagQuoted;
	while( parseTag( s, &itemTag, &itemTagQuoted, &tagSuffixComment ) ) {
		QSharedPointer<Item> item = parseItem( s, itemTag );
		s.skipUntilChars( ";}" );
		item->setTagSuffixComment( tagSuffixComment );
		if( itemTagQuoted )
			item->setTagQuoted();
		result->addItem( item );
		s.readChar();
	}

	return result;
}

QSharedPointer<PList::Array> PList::parseArray( TextStream &s, const QString &tag )
{
	QSharedPointer<PList::Array> result( new PList::Array( tag ) );

	bool done = false;
	QString itemTag;
	while( ! done ) {
		QSharedPointer<PList::Item> item = parseItem( s, itemTag );
		if( item ) {
			result->addItem( item );
			s.skipUntilChars( ",)" );
			s.readChar();
		}
		else
			done = true;
	}

	return result;
}

void PList::Item::printTag( std::ostream &os ) const
{
	if( ! mTag.isEmpty() ) {
		if( needsQuotes( mTag ) || mTagQuoted )
			os << "\"" << qPrintable(mTag) << "\"";
		else
			os << qPrintable(mTag);
		if( ! mTagSuffixComment.isEmpty() )
			os << " " << qPrintable(mTagSuffixComment);
		os << " = ";
	}
}

bool PList::Item::sdkMatches( const QString &sdk ) const
{
	if( sdk == "*" )
		return true;
	
	int idx = mTag.indexOf( "[sdk=" );
	if( idx == -1 )
		return sdk == "";
	int endIdx = mTag.indexOf( ']', idx );
	if( endIdx == -1 )
		return sdk == "";
	idx += QString("[sdk=").length();
	QString thisSdk = mTag.mid( idx, endIdx - idx );
	return thisSdk == sdk;
}

void PList::Item::printValueSuffix( std::ostream &os ) const
{
	if( ! mValueSuffixComment.isEmpty() )
		os << " " << qPrintable( mValueSuffixComment );
}

QString PList::Item::quotedString( const QString &s )
{
	QString quoted = s;
	quoted.replace( '\"', "\\\"" );
	quoted.prepend( '"' );
	quoted.append( '"' );
	return quoted;
}

void PList::ValueString::print( std::ostream &os, int tabs ) const
{
	printTabs( os, tabs);
	printTag( os );
	if( mQuotes || needsQuotes( mValue ) )
		os << qPrintable(quotedString( mValue ));
	else
		os << qPrintable(mValue);
	printValueSuffix( os );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PList::Dictionary
PList::Dictionary::Dictionary( const Dictionary &rhs )
	: Item( rhs )
{
	for( QList<ItemRef>::ConstIterator itemIt = rhs.mItems.begin(); itemIt != rhs.mItems.end(); ++itemIt )
		mItems.push_back( (*itemIt)->clone() );
}

QString& PList::Dictionary::operator[]( const QString &name )
{
	QSharedPointer<PList::ValueString> valPtr = get<PList::ValueString>( name );
	if( valPtr )
		return valPtr->mValue;
	else {
		QSharedPointer<PList::ValueString> vs = ValueString::create( name, "", false );
		push_back( vs );
		return vs->mValue;
	}
}

const QString& PList::Dictionary::getValue( const QString &name, const QString &defaultValue ) const
{
	QSharedPointer<PList::ValueString> valPtr = get<PList::ValueString>( name );
	if( valPtr )
		return valPtr->getValue();
	else
		return defaultValue;
}

void PList::Dictionary::replaceItem( QSharedPointer<Item> oldItem, QSharedPointer<Item> newItem )
{
	QList<QSharedPointer<Item> >::Iterator oldItemIt = std::find( mItems.begin(), mItems.end(), oldItem );
	if( oldItemIt != mItems.end() ) {
		mItems.replace( oldItemIt - mItems.begin(), newItem );
	}
	else
		mItems.push_back( newItem );
}

void PList::Dictionary::erase( const QString &name )
{
	for( QList<QSharedPointer<Item> >::iterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
		if( (*itIt)->getTag() == name ) {
			mItems.erase( itIt );
			break;
		}
	}
}

void PList::Dictionary::print( std::ostream &os, int tabs ) const
{
	printTabs( os, tabs );
	printTag( os );
	os << "{";
	if( mCarriageReturnsOnContents )
		os << std::endl;
	for( QList<QSharedPointer<PList::Item> >::ConstIterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
		if( mCarriageReturnsOnContents )
			(*itIt)->print( os, tabs + 1 );
		else
			(*itIt)->print( os, 0 );
		if( ! (*itIt)->isComment() )
			os << ";";
		if( mCarriageReturnsOnContents )
			os << std::endl;
		else
			os << " ";
	}
	if( mCarriageReturnsOnContents )
		printTabs( os, tabs );
	os << "}";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PList::Array
PList::Array::Iterator PList::Array::findValue( const QString &value )
{
	for( QList<QSharedPointer<PList::Item> >::iterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
		QSharedPointer<PList::ValueString> valPtr = itIt->dynamicCast<PList::ValueString>();
		if( valPtr && valPtr->getValue() == value ) {
			return itIt;
		}
	}

	return mItems.end();
}

PList::Array::Array( const Array &rhs )
	: Item( rhs )
{
	for( QList<ItemRef>::ConstIterator itemIt = rhs.mItems.begin(); itemIt != rhs.mItems.end(); ++itemIt )
		mItems.push_back( (*itemIt)->clone() );
}

void PList::Array::print( std::ostream &os, int tabs ) const
{
	printTabs( os, tabs );
	printTag( os );
	os << "(" << std::endl;
	for( QList<QSharedPointer<PList::Item> >::ConstIterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
		(*itIt)->print( os, tabs + 1 );
		os << "," << std::endl;
	}
	printTabs( os, tabs );
	os << ")";
}

void PList::Array::addItem( QSharedPointer<Item> item )
{
	mItems.push_back( item );
}