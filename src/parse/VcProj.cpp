#include "VcProj.h"
#include "Util.h"
#include <QUuid>

VcProj::VcProj( const QString &vcProjString, const QString &vcProjFiltersString )
{
	// .vcxproj
	mProjDom = QSharedPointer<pugi::xml_document>( new pugi::xml_document() );
	std::string vcProjStdStr = vcProjString.toStdString();
	pugi::xml_parse_result result = mProjDom->load_buffer( vcProjStdStr.c_str(), vcProjStdStr.length() );
	if( ! result ) {
		throw VcProjExc( "Failed to parse VcProj" );
	}

	// .vcxproj.filters
	QSharedPointer<pugi::xml_document> projFiltersDom( new pugi::xml_document() );
	std::string vcProjFiltersStdStr = vcProjFiltersString.toStdString();
	result = projFiltersDom->load_buffer( vcProjFiltersStdStr.c_str(), vcProjFiltersStdStr.length() );
	if( ! result ) {
		throw VcProjExc( "Failed to parse VcProj filters" );
	}
	mFilters = QSharedPointer<Filters>( new Filters( projFiltersDom ) );
}

void VcProj::setupNew( const QString &name, const std::vector<VcProj::ProjectConfiguration> &projectConfigurations, bool slnDeploy, bool useRcFile )
{
	mProjectConfigurations = projectConfigurations;
	mSlnDeploy = slnDeploy;
    mUseRcFile = useRcFile;
	mProjGuid = QUuid::createUuid().toString().toUpper();
	pugi::xpath_node projectGuidXPath = mProjDom->select_single_node( "/Project/PropertyGroup/ProjectGuid" );
	if( projectGuidXPath ) {
		pugi::xml_node projectGuidNode = projectGuidXPath.node().first_child();
		projectGuidNode.set_value( mProjGuid.toUtf8().constData() );
	}
	else
		throw VcProjExc( "Unable to locate <ProjectGuid> node" );

	removeUnusedProjectConfigurations();

	mProjName = name;
	pugi::xpath_node projectRoot = mProjDom->select_single_node( "/Project/PropertyGroup/RootNamespace" );
	if( projectRoot )
		projectRoot.node().first_child().set_value( mProjName.toUtf8().constData() );

	// create filters dom
	if( ! mFilters )
		mFilters = QSharedPointer<Filters>( new Filters() );
}

void VcProj::removeProjectConfiguration( const ProjectConfiguration &config )
{
	// delete the <ProjectConfiguration> itself
	std::string projectConfigurationNodesXPath = "/Project/ItemGroup[@Label=\"ProjectConfigurations\"]/ProjectConfiguration";
	pugi::xpath_node_set configurationNodes = mProjDom->select_nodes( projectConfigurationNodesXPath.c_str() );
	for( pugi::xpath_node_set::const_iterator it = configurationNodes.begin(); it != configurationNodes.end(); ++it ) {
		if( QString( it->node().attribute( "Include" ).value() ) == config.asString() ) {
			it->node().parent().remove_child( it->node() );
			break;
		}
	}

	// delete all nodes of the form
	// Condition="'$(Configuration)|$(Platform)'=='Debug_ANGLE|Win32'"
	std::string conditionString = "'$(Configuration)|$(Platform)'=='" + config.asString().toStdString() + "'";
	std::string conditionXPath = "//*[@Condition=\"" + conditionString + "\"]";
	pugi::xpath_node_set conditionNodes = mProjDom->select_nodes( conditionXPath.c_str() );
	for( pugi::xpath_node_set::const_iterator it = conditionNodes.begin(); it != conditionNodes.end(); ++it ) {
		it->node().parent().remove_child( it->node() );
	}
}

void VcProj::removeUnusedProjectConfigurations()
{
	std::vector<ProjectConfiguration> configsToBeDeleted;
	// iterate the vcxproj's 'ProjectConfigurations'
	pugi::xpath_node_set itemGroup = mProjDom->select_nodes("/Project/ItemGroup[@Label=\"ProjectConfigurations\"]/ProjectConfiguration");
	for( pugi::xpath_node_set::const_iterator it = itemGroup.begin(); it != itemGroup.end(); ++it ) {
		pugi::xpath_node node = it->node();
		pugi::xpath_node config = it->node().child( "Configuration" );
		ProjectConfiguration parsedConfig( it->node().child( "Configuration" ).first_child().value(), it->node().child( "Platform" ).first_child().value() );
		bool found = false;
		for( const auto &config : mProjectConfigurations ) {
			if( config == parsedConfig ) {
				found = true;
				break;
			}
		}
		if( ! found ) {
			configsToBeDeleted.push_back( parsedConfig );
			std::cout << "Removing " << qPrintable( mProjectConfigurations.back().asString() ) << std::endl;
		}
		else {
			std::cout << "Keeping " << qPrintable( mProjectConfigurations.back().asString() ) << std::endl;
		}
	}

	for( auto &doomedConfig : configsToBeDeleted )
		removeProjectConfiguration( doomedConfig );
}

void VcProj::addSourceFile( const QString &fileSystemPath, const QString &virtualPath )
{
	pugi::xml_node sourceGroup = getSourceItemGroup();
	sourceGroup.append_child( "ClCompile" ).append_attribute( "Include" ) = fileSystemPath.toUtf8().constData();
	mFilters->addSourceFile( fileSystemPath.toStdString(), virtualPath.toStdString() );
}

void VcProj::addHeaderFile( const QString &fileSystemPath, const QString &virtualPath, bool isResourcesHeader )
{
	pugi::xml_node headerGroup = getHeaderItemGroup();
	headerGroup.append_child( "ClInclude" ).append_attribute( "Include" ) = fileSystemPath.toUtf8().constData();
	mFilters->addHeaderFile( fileSystemPath.toStdString(), virtualPath.toStdString() );

	if( isResourcesHeader )
		mResourcesHeaderPath = fileSystemPath;
}

void VcProj::addStaticLibrary( const QString &config, const QString &path )
{
	for( auto &projConfig : mProjectConfigurations ) {
pugi::xml_node defGroup = findItemDefinitionGroup( projConfig );
		pugi::xml_node link = defGroup.child( "Link" );
		pugi::xml_node additionalDependencies = link.child( "AdditionalDependencies" );
		appendToDelimitedList( &additionalDependencies, path, ";" );
	}
}

void VcProj::addBuildCopy( const QString &config, const QString &path )
{
	appendPostBuildCommand( config, QString("xcopy /y \"") + path + QString("\" \"$(OutDir)\"" ) );
}

void VcProj::appendPostBuildCommand( const QString &config, const QString &command )
{
	for( auto &projConfig : mProjectConfigurations ) {
pugi::xml_node defGroup = findItemDefinitionGroup( projConfig );
		pugi::xml_node postBuild = defGroup.child( "PostBuildEvent" );
		if( postBuild.empty() )
			postBuild = defGroup.append_child( "PostBuildEvent" );
		pugi::xml_node commandNode = postBuild.child( "Command" );
		if( commandNode.empty() )
			commandNode = postBuild.append_child( "Command" );
		appendToDelimitedList( &commandNode, command, "\n\r" );
	}
}

void VcProj::addHeaderPath( const QString &config, const QString &path )
{
	for( auto &projConfig : mProjectConfigurations ) {
pugi::xml_node defGroup = findItemDefinitionGroup( projConfig );
		pugi::xml_node clCompile = defGroup.child( "ClCompile" );
		pugi::xml_node additionalInclude = clCompile.child( "AdditionalIncludeDirectories" );
		appendToDelimitedList( &additionalInclude, path, ";" );
	}
}

void VcProj::addLibraryPath( const QString &config, const QString &path )
{
	for( auto &projConfig : mProjectConfigurations ) {
pugi::xml_node defGroup = findItemDefinitionGroup( projConfig );
		pugi::xml_node clCompile = defGroup.child( "Link" );
		pugi::xml_node additionalLib = clCompile.child( "AdditionalLibraryDirectories" );
		appendToDelimitedList( &additionalLib, path, ";" );
	}
}

void VcProj::addResourceFile( const QString &name, const QString &fileSystemPath, const QString &type, int id )
{
	if( mUseRcFile ) {
		if( id == -1 ) { // auto-generate resource ID
			bool found;
			id = 1;
			do {
				found = false;
				for( QList<Resource>::ConstIterator resIt = mRcResources.begin(); resIt != mRcResources.end(); ++resIt ) {
					if( resIt->mId == id && resIt->mType == type ) {
						++id;
						found = true;
						break;
					}
				}
			} while( found );
		}

		mRcResources.push_back( VcProj::Resource( name, fileSystemPath, type, id ) );
	}
	else {
		pugi::xml_node resourceGroup = getResourceItemGroup();
		pugi::xml_node item = resourceGroup.append_child( "None" );
		item.append_attribute( "Include" ) = fileSystemPath.toUtf8().constData();
		item.append_child( "DeploymentContent" ).append_child(pugi::node_pcdata).set_value( "true" );
		mFilters->addResourceFile( fileSystemPath.toStdString(), ""/*virtualPath.toStdString()*/ );
	}
}

void VcProj::addPreprocessorDefine( const QString &config, const QString &value )
{
	for( auto &projConfig : mProjectConfigurations ) {
pugi::xml_node defGroup = findItemDefinitionGroup( projConfig );
		pugi::xml_node clCompile = defGroup.child( "ClCompile" );
		pugi::xml_node additionalInclude = clCompile.child( "PreprocessorDefinitions" );
		appendToDelimitedList( &additionalInclude, value, ";" );
	}
}

bool VcProj::nodeConditionsMatch( const pugi::xml_node &node, const QString &config, const QString &platform )
{
	QString condString = QString::fromUtf8( node.attribute( "Condition" ).value() );
	condString.replace( "$(Configuration)", config );
	condString.replace( "$(Platform)", platform );
	QStringList halves = condString.split( "==" );
	return halves.length() == 2 && halves[0] == halves[1];
}

std::string VcProj::getConditionString( const QString &config, const QString &platform )
{
	QString result = "'$(Configuration)|$(Platform)'=='";
	result += config;
	result += "|";
	result += platform;
	result += "'";
	return result.toStdString();
}

void VcProj::setTargetExtension( const QString &config, const QString &platform, const QString &originalExtension )
{
	QString extension = originalExtension;
	if( extension.isEmpty() )
		return;
	if( extension[0] != '.' )
		extension = QString(".") + extension;

	// see if the TargetExt node already exists with the right config
	pugi::xpath_node_set targetExtGroups = mProjDom->select_nodes("/Project/PropertyGroup/TargetExt");
	for( pugi::xpath_node_set::const_iterator it = targetExtGroups.begin(); it != targetExtGroups.end(); ++it ) {
		if( nodeConditionsMatch( it->node(), config, platform ) ) {
			it->node().first_child().set_value( extension.toUtf8().constData() );
			return;
		}
	}

	// didn't find a TargetExt node with the appropriate config; add it to the PropertyGroup with <OutDir>
	pugi::xpath_node propertyGroup = mProjDom->select_single_node("/Project/PropertyGroup[OutDir]");
	if( ! propertyGroup.node().empty() ) {
		pugi::xml_node newNode = propertyGroup.node().append_child( "TargetExt" );
		newNode.append_child(pugi::node_pcdata).set_value( extension.toUtf8().constData() );
		pugi::xml_attribute attrNode = newNode.append_attribute( "Condition" );
		attrNode.set_value( getConditionString( config, platform ).c_str() );
	}
}

pugi::xml_node VcProj::findItemDefinitionGroup( const ProjectConfiguration &projConfig )
{
	return findItemDefinitionGroup( projConfig.getConfig(), projConfig.getPlatform() );
}

pugi::xml_node VcProj::findItemDefinitionGroup( const QString &config, const QString &platform )
{
	pugi::xpath_node_set itemDefGroups = mProjDom->select_nodes("/Project/ItemDefinitionGroup");
	for( pugi::xpath_node_set::const_iterator it = itemDefGroups.begin(); it != itemDefGroups.end(); ++it ) {
		if( nodeConditionsMatch( it->node(), config, platform ) )
			return it->node();
	}

	return pugi::xml_node(); // failure
}

void VcProj::appendToDelimitedList( pugi::xml_node *node, const QString &value, const QString &delimeters )
{
	QString curVal = (node->first_child()) ? node->first_child().value() : "";
	if( curVal.isEmpty() || curVal.endsWith( delimeters ) )
		curVal += value;
	else
		curVal += delimeters + value;
	if( node->first_child() )
		node->first_child().set_value( curVal.toUtf8().constData() );
	else
		node->append_child( pugi::node_pcdata ).set_value( curVal.toUtf8().constData() );
}

pugi::xml_node VcProj::getSourceItemGroup()
{
	if( mProjSourceItemGroup )
		return mProjSourceItemGroup;

	pugi::xpath_node projectRoot = mProjDom->select_single_node( "/Project/ItemGroup/ClCompile" );
	if( projectRoot ) {
		mProjSourceItemGroup = projectRoot.node().parent();
	}
	else {
		pugi::xml_node importNode = mProjDom->select_single_node( "/Project/Import[last()]" ).node();
		mProjSourceItemGroup = importNode.parent().insert_child_before( "ItemGroup", importNode );
	}

	return mProjSourceItemGroup;
}

// Only used for non-RC resources
pugi::xml_node VcProj::getResourceItemGroup()
{
	pugi::xpath_node projectRoot = mProjDom->select_single_node( "/Project/ItemGroup/AppxManifest" );
	if( projectRoot ) {
		return projectRoot.node().parent();
	}
	else {
		pugi::xml_node importNode = mProjDom->select_single_node( "/Project/Import[last()]" ).node();
		return importNode.parent().insert_child_before( "ItemGroup", importNode );
	}

	return mProjSourceItemGroup;
}

pugi::xml_node VcProj::getHeaderItemGroup()
{
	if( mProjHeaderItemGroup )
		return mProjHeaderItemGroup;

	pugi::xpath_node projectRoot = mProjDom->select_single_node( "/Project/ItemGroup/ClInclude" );
	if( projectRoot ) {
		mProjHeaderItemGroup = projectRoot.node().parent();
	}
	else {
		pugi::xml_node importNode = mProjDom->select_single_node( "/Project/Import[last()]" ).node();
		mProjHeaderItemGroup = importNode.parent().insert_child_before( "ItemGroup", importNode );
	}

	return mProjHeaderItemGroup;
}

QString VcProj::getSlnString() const
{
	QString result;

	result = getSlnHeaderString();
	result += "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" + getProjName();
			result += QString("\", \"") + getProjName() + ".vcxproj\", \"" + getProjGuid() + "\"\r\n";
	result += "EndProject\r\n";
	result += "Global\r\n";
		result += "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n";
			for( int i = 0; i < mProjectConfigurations.size(); ++i )
				result += "\t\t" + mProjectConfigurations[i].asString() + " = " + mProjectConfigurations[i].asString() + "\r\n";
		result += "\tEndGlobalSection\r\n";
		result += "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n";

		for( int i = 0; i < mProjectConfigurations.size(); ++i ) {
			const auto projConfig = mProjectConfigurations[i];
			result += "\t\t" + getProjGuid() + "." + projConfig.asString() + ".ActiveCfg = " + projConfig.asString() + "\r\n";
			result += "\t\t" + getProjGuid() + "." + projConfig.asString() + ".Build.0 = " + projConfig.asString() + "\r\n";
			if( mSlnDeploy )
				result += "\t\t" + getProjGuid() + "." + projConfig.asString() + ".Deploy.0 = " + projConfig.asString() + "\r\n";
		}

		result += "\tEndGlobalSection\r\n";
		result += "\tGlobalSection(SolutionProperties) = preSolution\r\n";
			result += "\t\tHideSolutionNode = FALSE\r\n";
		result += "\tEndGlobalSection\r\n";
	result += "EndGlobal\r\n";
	return result;
}

// Returns the RC (resources) file as a string
QString VcProj::getRcString() const
{
	QString result;

	result += "#include \"../include/Resources.h\"\r\n";
	result += "\r\n";
	for( QList<Resource>::ConstIterator resIt = mRcResources.begin(); resIt != mRcResources.end(); ++resIt ) {
		// an empty name means we just write the resource without invoking the CINDER_RESOURCE macro
		if( resIt->mName.isEmpty() ) {
			QString quotedPath = "\"" + resIt->getFilePath() + "\"";
			quotedPath.replace( "\\", "\\\\" );
			result += QString::number( resIt->getId() ) + "\t" + resIt->getType() + "\t" + quotedPath;
		}
		else {
			result += resIt->mName;
		}
		result += "\r\n";
	}

	return result;
}

void VcProj::write( const QString &directoryPath, const QString &namePrefix ) const
{
	QDir dir( directoryPath );

	if( ! mResourcesHeaderPath.isEmpty() ) {
		QString absResourcesHeaderPath = dir.absoluteFilePath( QFileInfo( mResourcesHeaderPath ).filePath() );
		absResourcesHeaderPath.replace( "\\", "/" );
		absResourcesHeaderPath = QDir::cleanPath( absResourcesHeaderPath );
//		writeResourceHeader( absResourcesHeaderPath );
	}

	struct xml_string_writer: pugi::xml_writer
	{
		std::string result;
		virtual void write(const void* data, size_t size) { result += std::string(static_cast<const char*>(data), size); }
	};

	{ // write vcxproj
		QString writePath = dir.absoluteFilePath( namePrefix + ".vcxproj" );
		QFile outFile( writePath );
		if( ! outFile.open( QIODevice::WriteOnly ) )
			throw VcProjExc( "Failed to write to " + writePath );

		QTextStream ts( &outFile );
		ts.setCodec( "UTF-8" );
		xml_string_writer writer;
		mProjDom->print( writer, "  " );
		QString str = QString::fromUtf8( writer.result.c_str() ).replace( "\n", "\r\n" );
		ts << str;
	}

	{ // write filters
		QString writePath = dir.absoluteFilePath( namePrefix + ".vcxproj.filters" );
		QFile outFile( writePath );
		if( ! outFile.open( QIODevice::WriteOnly ) )
			throw VcProjExc( "Failed to write to " + writePath );

		QTextStream ts( &outFile );
		ts.setCodec( "UTF-8" );
		xml_string_writer writer;
		mFilters->getDom()->print( writer, "  " );
		QString str = QString::fromUtf8( writer.result.c_str() ).replace( "\n", "\r\n" );
		ts << str;
	}

	{ // write sln
		QString writePath = dir.absoluteFilePath( namePrefix + ".sln" );
		QFile outFile( writePath );
		if( ! outFile.open( QIODevice::WriteOnly ) )
			throw VcProjExc( "Failed to write to " + writePath );

		QTextStream ts( &outFile );
		ts.setCodec( "UTF-8" );
		ts << getSlnString();
	}

	if( mUseRcFile ){ // write RC
		QString writePath = dir.absoluteFilePath( "Resources.rc" );
		QFile outFile( writePath );
		if( ! outFile.open( QIODevice::WriteOnly ) )
			throw VcProjExc( "Failed to write to " + writePath );

		QTextStream ts( &outFile );
		ts.setCodec( "Windows-1252" ); // this could be UTF-16LE but VC++ can't display it
		ts << getRcString();
	}
}

// Adds all the resources to our Resources.h file
/*void VcProj::writeResourceHeader( const QString &resourcesHeaderPath ) const
{
//TODO
return;
	QString output;
	if( ! resourcesHeaderPath.isEmpty() ) {
		{ // read and append
			QFile headerFile( resourcesHeaderPath );
			if( ! headerFile.open( QIODevice::ReadOnly ) )
				throw VcProjExc( "Failed to load Resources header: " + resourcesHeaderPath );

			QTextStream ts( &headerFile );
			ts.setCodec( "UTF-8" );
			output = ts.readAll();
			for( QList<Resource>::ConstIterator resIt = mResources.begin(); resIt != mResources.end(); ++resIt ) {
				output += "\r\n";
				if( ! resIt->mName.isEmpty() ) { // we don't add empty-named resources to the header
					output += "#define " + resIt->mName + "\t\t\tCINDER_RESOURCE( " + resIt->getFileDir() + "/" + ", " + resIt->getFileName() + ", "
						+ QString::number( resIt->mId ) + ", " + resIt->mType + " )";
				}
			}
			output += "\r\n";
		}

		{ // write
			QFile headerFile( resourcesHeaderPath );
			if( ! headerFile.open( QIODevice::WriteOnly ) )
				throw VcProjExc( "Failed to write Resources header: " + resourcesHeaderPath );

			QTextStream ts( &headerFile );
			ts.setCodec( "UTF-8" );
			ts << output;
		}
	}
	else
		throw VcProjExc( "No file marked as the Resources header." );
}
*/
///////////////////////////////////////////////////////////////////////////////////////////
// VcProj::Filters
VcProj::Filters::Filters()
{
	mDom = QSharedPointer<pugi::xml_document>( new pugi::xml_document() );
	pugi::xml_node declaration = mDom->append_child( pugi::node_declaration );
	declaration.append_attribute( "version" ) = "1.0";
	declaration.append_attribute( "encoding" ) = "utf-8";
	pugi::xml_node projectNode = mDom->append_child( "Project" );
	projectNode.append_attribute("ToolsVersion") = "4.0";
	projectNode.append_attribute("xmlns") = "http://schemas.microsoft.com/developer/msbuild/2003";

	// force creation of base filters by "finding" them
	findSourcesFilter();
	findHeadersFilter();
	findRcResourcesFilter();
}

VcProj::Filters::Filters( QSharedPointer<pugi::xml_document> dom )
	: mDom( dom )
{
	// force creation of base filters by "finding" them
	findSourcesFilter();
	findHeadersFilter();
	findRcResourcesFilter();
}

pugi::xml_node VcProj::Filters::findFiltersItemGroup()
{
	pugi::xml_node result = mDom->select_single_node( "Project/ItemGroup/Filter" ).parent();
	if( ! result )
		result = mDom->select_single_node("Project").node().append_child( "ItemGroup" );
	return result;
}

pugi::xml_node VcProj::Filters::findSourcesFilter()
{
	pugi::xpath_node result = mDom->select_single_node( "Project/ItemGroup/Filter/Extensions[contains(.,'cpp')]" );
	if( ! result )
		return addFilter( "Source Files", "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx" );
	else
		return result.parent();
}

pugi::xml_node VcProj::Filters::findHeadersFilter()
{
	pugi::xpath_node result = mDom->select_single_node( "Project/ItemGroup/Filter/Extensions[contains(.,'hpp')]" );
	if( ! result )
		return addFilter( "Header Files", "h;hpp;hxx;hm;inl;inc;xsd" );
	else
		return result.parent();
}

pugi::xml_node VcProj::Filters::findResourcesFilter()
{
	pugi::xpath_node result = mDom->select_single_node( "Project/ItemGroup/Filter/Extensions[contains(.,'rc')]" );
	if( ! result )
		return addFilter( "Resource Files", "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav" );
	else
		return result.parent();
}

pugi::xml_node VcProj::Filters::findRcResourcesFilter()
{
	pugi::xpath_node result = mDom->select_single_node( "Project/ItemGroup/Filter/Extensions[contains(.,'rc')]" );
	if( ! result )
		return addFilter( "Resource Files", "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav" );
	else
		return result.parent();
}

pugi::xml_node VcProj::Filters::findFilter( const std::string &filterName ) const
{
	std::string query = std::string("Project/ItemGroup/Filter[@Include = '") + filterName + "']";
	pugi::xpath_node result = mDom->select_single_node( query.c_str() );
	if( result )
		return result.node();
	else
		return pugi::xml_node();
}

// this assumes 'filterPath' is a file path, so it ignores the final component
pugi::xml_node VcProj::Filters::createAndFindFiltersPath( std::string filterPath )
{
	QStringList items = QString::fromUtf8( filterPath.c_str() ).split( '/' );
	QString curPath;
	pugi::xml_node result;
	for( QStringList::Iterator itemIt = items.begin(); (items.length() > 1) && (itemIt != (items.end()-1)); ++itemIt ) {
		curPath += *itemIt;
		result = findFilter( curPath.toStdString() );
		if( ! result ) {
			result = addFilter( curPath.toStdString(), "" );
		}
		curPath += '\\';
	}

	return result;
}

pugi::xml_node VcProj::Filters::findSourcesItemGroup()
{
	pugi::xpath_node result = mDom->select_single_node( "Project/ItemGroup/ClCompile" );
	if( ! result ) {
		return mDom->select_single_node( "Project" ).node().append_child( "ItemGroup" );
	}
	else
		return result.parent();
}

pugi::xml_node VcProj::Filters::findHeadersItemGroup()
{
	pugi::xpath_node result = mDom->select_single_node( "Project/ItemGroup/ClInclude" );
	if( ! result ) {
		return mDom->select_single_node( "Project" ).node().append_child( "ItemGroup" );
	}
	else
		return result.parent();
}

pugi::xml_node VcProj::Filters::addFilter( const std::string &filterName, const std::string &extensions )
{
	pugi::xml_node filtersItemGroup = findFiltersItemGroup();
	pugi::xml_node sourceFilter = filtersItemGroup.append_child( "Filter" );
		sourceFilter.append_attribute( "Include" ) = filterName.c_str();
			sourceFilter.append_child( "UniqueIdentifier" ).append_child(pugi::node_pcdata).set_value( QUuid::createUuid().toString().toUpper().toStdString().c_str() );
			if( ! extensions.empty() )
				sourceFilter.append_child( "Extensions" ).append_child(pugi::node_pcdata).set_value( extensions.c_str() );
	return sourceFilter;
}

void VcProj::Filters::addSourceFile( const std::string &fileSystemPath, const std::string &virtualPath )
{
	pugi::xml_node filter;
	if( virtualPath.empty() )
		filter = findSourcesFilter();
	else
		filter = createAndFindFiltersPath( virtualPath );
	pugi::xml_node item = findSourcesItemGroup().append_child( "ClCompile" );
		item.append_attribute( "Include" ) = fileSystemPath.c_str();
		item.append_child( "Filter" ).append_child(pugi::node_pcdata).set_value( filter.attribute( "Include" ).value() );
}

void VcProj::Filters::addHeaderFile( const std::string &fileSystemPath, const std::string &virtualPath )
{
	pugi::xml_node filter;
	if( virtualPath.empty() )
		filter = findHeadersFilter();
	else
		filter = createAndFindFiltersPath( virtualPath );
	pugi::xml_node item = findSourcesItemGroup().append_child( "ClInclude" );
	item.append_attribute( "Include" ) = fileSystemPath.c_str();
	item.append_child( "Filter" ).append_child(pugi::node_pcdata).set_value( filter.attribute( "Include" ).value() );
}

void VcProj::Filters::addResourceFile( const std::string &fileSystemPath, const std::string &virtualPath )
{
	pugi::xml_node filter;
	if( virtualPath.empty() )
		filter = findSourcesFilter();
	else
		filter = createAndFindFiltersPath( virtualPath );
	pugi::xml_node item = findSourcesItemGroup().append_child( "ClCompile" );
		item.append_attribute( "Include" ) = fileSystemPath.c_str();
		item.append_child( "Filter" ).append_child(pugi::node_pcdata).set_value( filter.attribute( "Include" ).value() );
}
