#include "EdgeDetectionRenderer.h"

#include "DX11RendererCore.h"

#include "EdgeDetectionComputeShader.h"
#include "EdgeDistanceComputeShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

EdgeDetectionRenderer::EdgeDetectionRenderer( DX11RendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_edgeDetectionComputeShader( std::make_shared< EdgeDetectionComputeShader >() ),
    m_edgeDistanceComputeShader( std::make_shared< EdgeDistanceComputeShader >() )
{}

EdgeDetectionRenderer::~EdgeDetectionRenderer()
{}

void EdgeDetectionRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device, 
                                          ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device        = device;
	this->m_deviceContext = deviceContext;

	this->m_imageWidth  = imageWidth;
	this->m_imageHeight = imageHeight;

    createRenderTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( device );

	m_initialized = true;
}

void EdgeDetectionRenderer::performEdgeDetection( const std::shared_ptr< Texture2D< float4 > > positionTexture,
                                                  const std::shared_ptr< Texture2D< float4 > > normalTexture )
{
    // For test - may not be necessary.
    m_valueRenderTargetSrc->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 0, 0, 0 ) );
    m_valueRenderTargetDest->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 0, 0, 0 ) );

    m_rendererCore.disableRenderingPipeline();

	RenderTargets unorderedAccessTargets;
    
    { // Mark edges with value of 0.
		unorderedAccessTargets.typeUchar.push_back( m_valueRenderTargetDest );

		m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

        m_edgeDetectionComputeShader->setParameters( *m_deviceContext.Get(), *positionTexture, *normalTexture );

        m_rendererCore.enableComputeShader( m_edgeDetectionComputeShader );

        uint3 groupCount( m_imageWidth / 8, m_imageHeight / 8, 1 );

        m_rendererCore.compute( groupCount );

        m_edgeDetectionComputeShader->unsetParameters( *m_deviceContext.Get() );
    }

    { // Calculate distance to nearest edge for each pixel - in 255 passes (because max dist is 255).
         m_rendererCore.enableComputeShader( m_edgeDistanceComputeShader );

         uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

         for ( int i = 1; i <= 255; ++i ) 
         {
             // Unset parameters to avoid binding the same resource twice.
             m_edgeDistanceComputeShader->unsetParameters( *m_deviceContext.Get() );

             // Swap source and destination texture.
             swapSrcDestRenderTargets();

             // Enable new destination render target.
			 unorderedAccessTargets.typeUchar.clear();
			 unorderedAccessTargets.typeUchar.push_back( m_valueRenderTargetDest );
			 m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

             m_edgeDistanceComputeShader->setParameters( *m_deviceContext.Get(), *m_valueRenderTargetSrc, (unsigned char)i );

             m_rendererCore.compute( groupCount );
         }
    }

    m_rendererCore.disableComputePipeline();
}

std::shared_ptr< RenderTargetTexture2D< unsigned char > > EdgeDetectionRenderer::getValueRenderTarget()
{
    return m_valueRenderTargetDest;
}

void EdgeDetectionRenderer::swapSrcDestRenderTargets()
{
    auto valueRenderTargetTemp = m_valueRenderTargetSrc;
    m_valueRenderTargetSrc       = m_valueRenderTargetDest;
    m_valueRenderTargetDest      = valueRenderTargetTemp;
}

void EdgeDetectionRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device3& device )
{
    m_valueRenderTargetDest = m_valueRenderTarget0 = std::make_shared< RenderTargetTexture2D< unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT );

    m_valueRenderTargetSrc = m_valueRenderTarget1 = std::make_shared< RenderTargetTexture2D< unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT );
}

void EdgeDetectionRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_edgeDetectionComputeShader->loadAndInitialize( "Engine1/Shaders/EdgeDetectionShader/EdgeDetection_cs.cso", device );
    m_edgeDistanceComputeShader->loadAndInitialize( "Engine1/Shaders/EdgeDistanceShader/EdgeDistance_cs.cso", device );
}
