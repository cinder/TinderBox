#pragma once

#include "Template.h"

class ProjectTemplate : public Template {
  public:
	ProjectTemplate()
		: Template()
	{}

	ProjectTemplate( const QString &parentPath, const pugi::xml_node &doc, ErrorList *errors );
    
    bool		hasParentProject() const { return ! mParentProjectId.isEmpty(); }
    QString		getParentProjectId() const { return mParentProjectId; }
    
  private:
	QString		mParentProjectId;
};
