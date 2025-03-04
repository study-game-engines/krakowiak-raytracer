#include "SkeletonAnimation.h"

#include <fstream>
#include <assert.h>
#include <algorithm>

#include "StringUtil.h"

#include "MyXAFFileParser.h"

#include "TextFile.h"

using namespace Engine1;

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::createFromFile( const std::string& path, const SkeletonAnimationFileInfo::Format format, const SkeletonMesh& mesh, const bool invertZCoordinate )
{
	std::shared_ptr< std::vector<char> > fileData = TextFile::load( path );

	std::shared_ptr<SkeletonAnimation> animation = createFromMemory( *fileData, format, mesh, invertZCoordinate );

	animation->getFileInfo().setPath( path );
	animation->getFileInfo().setFormat( format );
	animation->getFileInfo().setMeshFileInfo( mesh.getFileInfo() );
	animation->getFileInfo().setInvertZCoordinate( invertZCoordinate );

	return animation;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::createFromMemory( const std::vector<char>& fileData, const SkeletonAnimationFileInfo::Format format, const SkeletonMesh& mesh, const bool invertZCoordinate )
{
	std::shared_ptr<SkeletonAnimation> animation = std::make_shared<SkeletonAnimation>( );

	if ( SkeletonAnimationFileInfo::Format::XAF == format ) {
		MyXAFFileParser::parseSkeletonAnimationFile( fileData, mesh, *animation, invertZCoordinate );
	}

	return animation;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::calculateAnimationInSkeletonSpace( const SkeletonAnimation& animationInParentSpace, const SkeletonMesh& skeletonMesh )
{
	std::shared_ptr<SkeletonAnimation> animationInSkeletonSpace = std::make_shared<SkeletonAnimation>( );

	for ( const SkeletonPose& poseInParentSpace : animationInParentSpace.m_skeletonPoses )
		animationInSkeletonSpace->m_skeletonPoses.push_back( SkeletonPose::calculatePoseInSkeletonSpace( poseInParentSpace, skeletonMesh ) );

	return animationInSkeletonSpace;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::calculateAnimationInParentSpace( const SkeletonAnimation& animationInSkeletonSpace, const SkeletonMesh& skeletonMesh )
{
	std::shared_ptr<SkeletonAnimation> animationInParentSpace = std::make_shared<SkeletonAnimation>( );

	for ( const SkeletonPose& poseInSkeletonSpace : animationInSkeletonSpace.m_skeletonPoses )  
		animationInParentSpace->m_skeletonPoses.push_back( SkeletonPose::calculatePoseInParentSpace( poseInSkeletonSpace, skeletonMesh ) );

	return animationInParentSpace;
}

SkeletonAnimation::SkeletonAnimation()
{}

SkeletonAnimation::SkeletonAnimation( SkeletonAnimation&& other )
{
	// TODO: should be tested.
	m_skeletonPoses = std::move( other.m_skeletonPoses );
}

SkeletonAnimation::~SkeletonAnimation() 
{}

Asset::Type SkeletonAnimation::getType( ) const
{
	return Asset::Type::SkeletonAnimation;
}

std::vector< std::shared_ptr<const Asset> > SkeletonAnimation::getSubAssets( ) const
{
	return std::vector< std::shared_ptr<const Asset> >();
}

std::vector< std::shared_ptr<Asset> > SkeletonAnimation::getSubAssets( )
{
	return std::vector< std::shared_ptr<Asset> >();
}

void SkeletonAnimation::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	throw std::exception( "SkeletonAnimation::swapSubAsset - there are no sub-assets to be swapped." );
}

void SkeletonAnimation::setFileInfo( const SkeletonAnimationFileInfo& fileInfo )
{
	this->m_fileInfo = fileInfo;
}

const SkeletonAnimationFileInfo& SkeletonAnimation::getFileInfo( ) const
{
	return m_fileInfo;
}

SkeletonAnimationFileInfo& SkeletonAnimation::getFileInfo( )
{
	return m_fileInfo;
}

void SkeletonAnimation::addPose( SkeletonPose& pose ) 
{
	m_skeletonPoses.push_back( pose );
}

SkeletonPose SkeletonAnimation::getInterpolatedPose( float progress )
{
	const float frame               = progress * (float)( m_skeletonPoses.size() - 1 );
	const unsigned int prevKeyframe = std::max( 0u, (unsigned int)frame );
    const unsigned int nextKeyframe = std::min( (unsigned int)m_skeletonPoses.size( ) - 1, (unsigned int)frame + 1 );
	const float fraction            = frame - (float)prevKeyframe;

	if ( prevKeyframe == nextKeyframe )
		return m_skeletonPoses.at( prevKeyframe );
	else
		return SkeletonPose::blendPoses( m_skeletonPoses.at( prevKeyframe ), m_skeletonPoses.at( nextKeyframe ), fraction );
}

SkeletonPose& SkeletonAnimation::getPose( unsigned int keyframe )
{
	if ( keyframe >= m_skeletonPoses.size() ) throw std::exception( "SkeletonAnimation::getPose() - keyframe is out of range." );

	return m_skeletonPoses.at( keyframe );
}

SkeletonPose& SkeletonAnimation::getOrAddPose( unsigned int keyframe )
{
	while ( m_skeletonPoses.size( ) <= keyframe )
		m_skeletonPoses.push_back( SkeletonPose() );

	return m_skeletonPoses.at( keyframe );
}

unsigned int SkeletonAnimation::getKeyframeCount()
{
	return (unsigned int)m_skeletonPoses.size();
}
