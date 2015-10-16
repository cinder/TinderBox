/*
 Copyright (c) 2015, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
    the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <QList>
#include <QSharedPointer>
#include <QFile>

#include <iostream>

class PList
{
  public:
	class TextStream {
	  public:
		TextStream( QSharedPointer<class QTextStream> stream )
			: mStream( stream ), mCurrentLine( 1 ) {}

		QChar		readChar();
		QString		readLine();
		QString		readString();
		bool		readComment( QString *result = 0 );
		void		putChar( QChar c );
		bool		atEnd();

		void		skipUntilChar( QChar searchChar );
		void		skipUntilChars( QString searchChars, QString *comment = 0 );
		void		skipWhiteSpace( QString *comment = 0 );

		QList<QChar>						mOverflow;
		QSharedPointer<class QTextStream>	mStream;
		int									mCurrentLine;
	};

	class Item {
	  public:
		Item() {}
        Item( const QString &tag ) : mTag( tag ), mTagQuoted( false ), mCarriageReturnsOnContents( true ) {}
        Item( const Item &rhs ) : mTag( rhs.mTag ), mTagSuffixComment( rhs.mTagSuffixComment ),
				mValueSuffixComment( rhs.mValueSuffixComment ), mTagQuoted( rhs.mTagQuoted ), mCarriageReturnsOnContents( rhs.mCarriageReturnsOnContents )
		{}

		virtual ~Item() {}

		virtual QSharedPointer<Item>	clone() const = 0;

		QString			getTagSuffixComment() const { return mTagSuffixComment; }
		void			setTagSuffixComment( const QString &tagSuffixComment, bool autoCStyleComment = false ) { mTagSuffixComment = ( autoCStyleComment ) ? (QString("/* ") + tagSuffixComment + QString( " */" ) ) : tagSuffixComment; }
		QString			getValueSuffixComment() const { return mValueSuffixComment; }
		void			setValueSuffixComment( const QString &valueSuffixComment, bool autoCStyleComment = false ) { mValueSuffixComment = ( autoCStyleComment ) ? (QString("/* ") + valueSuffixComment + QString( " */" ) ) : valueSuffixComment; }
		const QString&	getTag() const { return mTag; }

		virtual void	print( std::ostream &os, int tabs ) const = 0;

		// should this Item when printed insert carriage returns between each of its children items
		void			setCarriageReturnsOnContents( bool cr = true ) { mCarriageReturnsOnContents = cr; }
		void			setTagQuoted() { mTagQuoted = true; }
		bool			isTagQuoted() const { return mTagQuoted; }

		bool			sdkMatches( const QString &sdk ) const;
		virtual bool	isComment() { return false; }

	  protected:
		static void		printTabs( std::ostream &os, int tabs ) { for( int t = 0; t < tabs; ++t ) os << "\t"; }
		void			printTag( std::ostream &os ) const;
		void			printValueSuffix( std::ostream &os ) const;
		static QString	quotedString( const QString &s );

		QString			mTag;
		QString			mTagSuffixComment, mValueSuffixComment;
		bool			mTagQuoted;
		bool			mCarriageReturnsOnContents;
	};
	typedef QSharedPointer<Item>	ItemRef;

	class LineComment : public Item {
	  public:
		LineComment( const QString &comment ) : Item( "" ), mComment( comment ) {}
		LineComment( const LineComment &rhs ) : Item( rhs ), mComment( rhs.mComment ) {}
		static QSharedPointer<LineComment>	create( const QString &comment ) { return QSharedPointer<LineComment>( new LineComment( comment ) ); }

		ItemRef	clone() const { return QSharedPointer<LineComment>( new LineComment( *this ) ); }

		virtual bool	isComment() { return true; }

		virtual void	print( std::ostream &os, int /*tabs*/ ) const {
			os << qPrintable( mComment );
		}

		QString			mComment;
	};

	class ValueString : public Item {
	  public:
		ValueString() : Item() {}
		ValueString( const QString &tag, const QString &value, bool quotes )
			: Item( tag ), mValue( value ), mQuotes( quotes ) {}
		ValueString( const ValueString &rhs )
			: Item( rhs ), mValue( rhs.mValue ), mQuotes( rhs.mQuotes )
		{}

		static QSharedPointer<ValueString>		create( const QString &tag, const QString &value, bool quotes = false ) { return QSharedPointer<ValueString>( new ValueString( tag, value, quotes ) ); }
		
		ItemRef	clone() const { return QSharedPointer<ValueString>( new ValueString( *this ) ); }

		const QString&	getValue() const { return mValue; }
		bool			isQuoted() const { return mQuotes; }

		void		print( std::ostream &os, int tabs ) const;

		QString			mValue;
		bool			mQuotes;
	};
	typedef QSharedPointer<ValueString>		ValueStringRef;

	class Array : public Item {
	  public:
		typedef QList<QSharedPointer<Item> >::Iterator Iterator;
		typedef QList<QSharedPointer<Item> >::ConstIterator ConstIterator;

		Array() : Item() {}
		Array( const QString &tag ) : Item( tag ) {}
		Array( const Array &rhs );

		static QSharedPointer<Array>	create( QString tag ) { return QSharedPointer<Array>( new Array( tag ) ); }

		ItemRef	clone() const { return QSharedPointer<Array>( new Array( *this ) ); }

		void		addItem( QSharedPointer<Item> item );

		void		print( std::ostream &os, int tabs ) const;

		Iterator		begin() { return mItems.begin(); }
		Iterator		end() { return mItems.end(); }
		Iterator		findValue( const QString &value );

		QList<ItemRef>	mItems;
	};
	typedef QSharedPointer<Array>	ArrayRef;

	class Dictionary : public Item {
	  public:
		typedef QList<QSharedPointer<Item> >::Iterator Iterator;

		Dictionary() : Item() {}
		Dictionary( const QString &tag ) : Item( tag ) {}
		Dictionary( const Dictionary &rhs );

		ItemRef	clone() const { return QSharedPointer<Dictionary>( new Dictionary( *this ) ); }

		QList<ItemRef>&		getItems() { return mItems; }
		void		addItem( QSharedPointer<Item> item ) { mItems.push_back( item ); }
		void		replaceItem( QSharedPointer<Item> oldItem, QSharedPointer<Item> newItem );
		void		erase( const QString &name );

		template<typename T>
		QSharedPointer<T>	get( const QString &name ) {
			for( QList<QSharedPointer<Item> >::iterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
				if( (*itIt)->getTag() == name ) {
					return itIt->dynamicCast<T>();
				}
			}

			return QSharedPointer<T>();
		}

		template<typename T>
		const QSharedPointer<T>	get( const QString &name ) const {
			for( QList<QSharedPointer<Item> >::const_iterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
				if( (*itIt)->getTag() == name ) {
					return itIt->dynamicCast<T>();
				}
			}

			return QSharedPointer<T>();
		}

		QList<ItemRef>	getSettings( const QString &name ) const { // allows for an sdk setting LIKE_THIS[sdk=iphonesimulator*]; returns multiple matches
			QList<ItemRef> result;
			for( QList<QSharedPointer<Item> >::const_iterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
				const QString s = (*itIt)->getTag();
				if( ( s == name ) || ( s.startsWith( name ) && ( s[name.length()] == '[' ) ) ) {
					ItemRef item = itIt->dynamicCast<Item>();
					if( item )
						result.push_back( item );
				}
			}

			return result;
		}

		bool contains( const QString &name ) const {
			for( QList<QSharedPointer<Item> >::const_iterator itIt = mItems.begin(); itIt != mItems.end(); ++itIt ) {
				if( (*itIt)->getTag() == name ) {
					return true;
				}
			}

			return false;
		}

		QString&	operator[]( const QString &name );
		const QString&	getValue( const QString &name, const QString &defaultValue ) const;

		void		print( std::ostream &os, int tabs ) const;

		void			push_back( QSharedPointer<Item> item ) { mItems.push_back( item ); }
		Iterator		begin() { return mItems.begin(); }
		Iterator		end() { return mItems.end(); }
		size_t			size() const { return mItems.size(); }

		QList<QSharedPointer<Item> >	mItems;
	};
	typedef QSharedPointer<PList::Dictionary>		DictionaryRef;

	static QSharedPointer<PList>		create( QFile &file ) { return QSharedPointer<PList>( new PList( file ) ); }
	static QSharedPointer<PList>		create( const QString &s ) { return QSharedPointer<PList>( new PList( s ) ); }

	class ParseFailed {
	};

	void		print( std::ostream &os ) { os << "// !$*UTF8*$!" << std::endl; mContents->print( os, 0 ); os << std::endl; }
	QSharedPointer<Item>		getContents() { return mContents; }

	// Returns whether a particular string needs to be quoted
	static bool			needsQuotes( const QString &str );

	void	insertBefore( QSharedPointer<Item> before, QSharedPointer<Item> value );
	void	insertAfter( QSharedPointer<Item> after, QSharedPointer<Item> value );

  protected:
	PList() {}
	PList( QFile &file );
	PList( const QString &s );

	void								parse( QFile &file );
	void								parse( const QString &s );
	QChar								parseUntilChar( TextStream &s, QChar c );
	QChar								parseNonWhitespaceChar( TextStream &s, QString *comment = 0 );
	void								parseComment( TextStream &s );
	bool								parseTag( TextStream &s, QString *result, bool *resultQuoted, QString *resultSuffixComment );
	bool								parseValue( TextStream &s, QString *result, QString *resultSuffixComment = 0 );
	bool								parseQuotedString( TextStream &s, QString *result );
	QSharedPointer<Item>				parseItem( TextStream &s, const QString &tag );
	QSharedPointer<PList::Dictionary>	parseDictionary( TextStream &s, const QString &tag );
	QSharedPointer<PList::Array>		parseArray( TextStream &s, const QString &tag );

	static bool							insertBeforeHelper( QSharedPointer<Item> parent, QSharedPointer<Item> before, QSharedPointer<Item> value );
	static bool							insertAfterHelper( QSharedPointer<Item> parent, QSharedPointer<Item> before, QSharedPointer<Item> value );

	QSharedPointer<Item>		mContents;
};
