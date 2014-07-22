#include "CinderBlock.h"

CinderBlock::CinderBlock( const QString &dir, const pugi::xml_node &xml, ErrorList *errors )
	: Template( dir, xml, errors ), mRequired( false ), mInstallType( INSTALL_NONE )
{
	mAuthor = QString::fromUtf8( xml.attribute( "author" ).value() );
	mGitUrl = QString::fromUtf8( xml.attribute( "git" ).value() );
	mBlockUrl = QString::fromUtf8( xml.attribute( "url" ).value() );
	mDescription = QString::fromUtf8( xml.attribute( "summary" ).value() );
	mLicense = QString::fromUtf8( xml.attribute( "license" ).value() );
	mLibraryUrl = QString::fromUtf8( xml.attribute( "library" ).value() );
	mVersion = QString::fromUtf8( xml.attribute( "version" ).value() );
}
