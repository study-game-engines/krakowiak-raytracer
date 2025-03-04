#include "Texture2DFileInfoParser.h"

#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

#include "AssetPathManager.h"
#include "FileUtil.h"

#include "Settings.h"

using namespace Engine1;

std::shared_ptr<Texture2DFileInfo> Texture2DFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<Texture2DFileInfo> fileInfo = std::make_shared<Texture2DFileInfo>( );

    const int fileNameSize = BinaryFile::readInt( dataIt );
    auto fileName = BinaryFile::readText( dataIt, fileNameSize );

    // Replace empty file name with default white texture.
    // Useful to handle textures which are empty, but contain color multipliers.
    if ( fileName.empty() )
    {
        fileName = ( fileInfo->getPixelType() == Texture2DFileInfo::PixelType::UCHAR4 )
            ? settings().importer.defaultWhiteUchar4TextureFileName 
            : "";
    }

    const auto filePath = AssetPathManager::get().getPathForFileName( fileName );

	fileInfo->setPath( filePath );
	fileInfo->setFormat( static_cast<Texture2DFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
    fileInfo->setPixelType( static_cast<Texture2DFileInfo::PixelType>( BinaryFile::readInt( dataIt ) ) );

	return fileInfo;
}

void Texture2DFileInfoParser::writeBinary( std::vector<char>& data, const Texture2DFileInfo& fileInfo )
{
    const auto fileName = FileUtil::getFileNameFromPath( fileInfo.getPath() );

	BinaryFile::writeInt(  data, (int)fileName.size( ) );
	BinaryFile::writeText( data, fileName );
	BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getFormat( ) ) );
    BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getPixelType( ) ) );
}