#include "AntialiasingRenderer.h"

#include "DX11RendererCore.h"

#include "ComputeShader.h"
#include "AntialiasingComputeShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

AntialiasingRenderer::AntialiasingRenderer( DX11RendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_luminanceComputeShader( std::make_shared< ComputeShader >() ),
    m_antialiasingComputeShader( std::make_shared< AntialiasingComputeShader >() )
{}

AntialiasingRenderer::~AntialiasingRenderer()
{}

void AntialiasingRenderer::initialize( ComPtr< ID3D11Device3 > device,
                                      ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void AntialiasingRenderer::calculateLuminance( std::shared_ptr< RenderTargetTexture2D< uchar4 > > texture )
{
    m_rendererCore.disableRenderingPipeline();

    m_rendererCore.enableComputeShader( m_luminanceComputeShader );

	RenderTargets unorderedAccessTargets;

	unorderedAccessTargets.typeUchar4.push_back( texture );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth = texture->getWidth();
    const int imageHeight = texture->getHeight();

    uint3 groupCount(
        imageWidth / 16,
        imageHeight / 16,
        1
    );

    m_rendererCore.compute( groupCount );

    m_rendererCore.disableComputePipeline();
}

void AntialiasingRenderer::performAntialiasing( 
    std::shared_ptr< Texture2D< uchar4 > > srcTexture,
    std::shared_ptr< RenderTargetTexture2D< uchar4 > > dstTexture )
{
    m_rendererCore.disableRenderingPipeline();

    m_rendererCore.enableComputeShader( m_antialiasingComputeShader );

    static float fxaaQualitySubpix           =  0.75f;
    static float fxaaQualityEdgeThreshold    = 0.166f;
    static float fxaaQualityEdgeThresholdMin = 0.0833f;

    m_antialiasingComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        *srcTexture, 
        fxaaQualitySubpix,
        fxaaQualityEdgeThreshold,
        fxaaQualityEdgeThresholdMin 
    );

	RenderTargets unorderedAccessTargets;

	unorderedAccessTargets.typeUchar4.push_back( dstTexture );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth = dstTexture->getWidth();
    const int imageHeight = dstTexture->getHeight();

    uint3 groupCount(
        imageWidth / 16,
        imageHeight / 16,
        1
    );

    m_rendererCore.compute( groupCount );

    m_antialiasingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void AntialiasingRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_luminanceComputeShader->loadAndInitialize( "Engine1/Shaders/AntialiasingShader/Luminance_cs.cso", device );
    m_antialiasingComputeShader->loadAndInitialize( "Engine1/Shaders/AntialiasingShader/Antialiasing_cs.cso", device );
}



