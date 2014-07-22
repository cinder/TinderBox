#include "ProjectTemplate.h"

#include <QString>

ProjectTemplate::ProjectTemplate( const QString &parentPath, const pugi::xml_node &doc, ErrorList *errors )
	: Template( parentPath, doc, errors )
{
	mParentProjectId = doc.attribute( "parent" ).value();
}

