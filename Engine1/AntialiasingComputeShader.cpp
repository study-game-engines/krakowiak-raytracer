#include "AntialiasingComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

AntialiasingComputeShader::AntialiasingComputeShader() {}

AntialiasingComputeShader::~AntialiasingComputeShader() {}

void AntialiasingComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    { // Create sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MipLODBias       = 0.0f;
        desc.MaxAnisotropy    = 1;
        desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
        desc.BorderColor[ 0 ] = 0;
        desc.BorderColor[ 1 ] = 0;
        desc.BorderColor[ 2 ] = 0;
        desc.BorderColor[ 3 ] = 0;
        desc.MinLOD           = 0;
        desc.MaxLOD           = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        HRESULT result = device->CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "GenerateMipmapMinValueComputeShader::compileFromFile - Failed to create texture sampler state." );
    }

    { // Create constant buffer.
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth           = sizeof( ConstantBuffer );
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 )
            throw std::exception( "AntialiasingComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void AntialiasingComputeShader::setParameters( 
    ID3D11DeviceContext3& deviceContext,
    Texture2D< uchar4 >& srcTexture,
    const float fxaaQualitySubpix,
    const float fxaaQualityEdgeThreshold,
    const float fxaaQualityEdgeThresholdMin )
{
    if ( !m_compiled )
        throw std::exception( "AntialiasingComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 1;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            srcTexture.getShaderResourceView(),
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 )
        throw std::exception( "AntialiasingComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->pixelSizeInTexcoords        = float2::ONE / (float2)srcTexture.getDimensions();
    dataPtr->fxaaQualitySubpix           = fxaaQualitySubpix;
    dataPtr->fxaaQualityEdgeThreshold    = fxaaQualityEdgeThreshold;
    dataPtr->fxaaQualityEdgeThresholdMin = fxaaQualityEdgeThresholdMin;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    { // Set texture sampler.
        deviceContext.CSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
    }
}

void AntialiasingComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled )
        throw std::exception( "AntialiasingComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 1, nullResources );

    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSamplers );
}
