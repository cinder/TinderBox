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

#include "GeneratorLinuxCmake.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "CmakeProj.h"

GeneratorLinuxCmake::GeneratorLinuxCmake()
{
}

QMap<QString,QString> GeneratorLinuxCmake::getConditions() const
{
    QMap<QString,QString> conditions;
    conditions["compiler"] = "clang";
    conditions["os"] = "linux";
    return conditions;
}

void GeneratorLinuxCmake::generate( Instancer *master )
{
    QMap<QString,QString> conditions = getConditions();
    conditions["config"] = "*";
    QList<Template::File> files = master->getFilesMatchingConditions( conditions );

    auto projectConfigurations = getConditions();

    QString absDirPath = master->createDirectory( "linux" );
    QString cinderPath = master->getMacRelCinderPath( absDirPath );

    // Load the foundation files as strings; replace variables appropriately
    QString replacedCmakeProj = loadAndStringReplace( ProjectTemplateManager::getFoundationPath( "linux_cmake/CMakeLists.txt" ),
        master->getNamePrefix(), cinderPath );
    auto cmakeProj = CmakeProj::createFromString( replacedCmakeProj );

    for( QList<Template::File>::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
        if( fileIt->getType() == Template::File::SOURCE )
            cmakeProj->addSourceFile( fileIt->getPosixOutputPathRelativeTo( absDirPath, cinderPath ), fileIt->getVirtualPath() );
    }

    cmakeProj->write( absDirPath );
}
