#include "RasterizingShadowsComputeShader.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"
#include "SpotLight.h"

#include "MathUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;
using namespace Engine1;

RasterizingShadowsComputeShader::RasterizingShadowsComputeShader() {}

RasterizingShadowsComputeShader::~RasterizingShadowsComputeShader() {}

void RasterizingShadowsComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    {
        // Create constant buffer.
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth           = sizeof( ConstantBuffer );
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "RasterizingShadowsComputeShader::compileFromFile - creating constant buffer failed." );
    }

    { // Create point sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
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
        HRESULT result = device->CreateSamplerState( &desc, m_pointSamplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "RasterizingShadowsComputeShader::compileFromFile - Failed to create texture sampler state." );
    }

    { // Create linear sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
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
        HRESULT result = device->CreateSamplerState( &desc, m_linearSamplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "RasterizingShadowsComputeShader::compileFromFile - Failed to create texture sampler state." );
    }

    this->m_device   = device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void RasterizingShadowsComputeShader::setParameters(
    ID3D11DeviceContext3& deviceContext,
    const float3& cameraPos,
    const Light& light,
    const Texture2D< float4 >& rayOriginTexture,
    const Texture2D< float4 >& surfaceNormalTexture,
    const int outputTextureWidth, const int outputTextureHeight )
{
    if ( !m_compiled ) 
        throw std::exception( "RasterizingShadowsComputeShader::setParameters - Shader hasn't been compiled yet." );

    std::shared_ptr< Texture2D< float > > shadowMap;

    if ( light.getType() == Light::Type::SpotLight )
        shadowMap = static_cast<const SpotLight&>( light ).getShadowMap();


    { // Set input buffers and textures.
        const unsigned int resourceCount = 3;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            rayOriginTexture.getShaderResourceView(),
            surfaceNormalTexture.getShaderResourceView(),
            shadowMap ? shadowMap->getShaderResourceView() : nullptr,
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) 
            throw std::exception( "RasterizingShadowsComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->outputTextureSize  = float2( (float)outputTextureWidth, (float)outputTextureHeight );
        dataPtr->lightPosition      = light.getPosition();
        dataPtr->lightEmitterRadius = light.getEmitterRadius();

        const SpotLight& spotLight = static_cast<const SpotLight&>( light );

        dataPtr->shadowMapViewMatrix       = spotLight.getShadowMapViewMatrix().getTranspose();
        dataPtr->shadowMapProjectionMatrix = spotLight.getShadowMapProjectionMatrix().getTranspose();
        dataPtr->lightConeMinDot           = cos( spotLight.getConeAngle() );
        dataPtr->lightDirection            = spotLight.getDirection();
        dataPtr->cameraPosition            = cameraPos;

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }

    { // Set texture samplers.
        ID3D11SamplerState* samplerStates[] = { m_pointSamplerState.Get(), m_linearSamplerState.Get() };
        deviceContext.CSSetSamplers( 0, 2, samplerStates );
    }
}

void RasterizingShadowsComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) 
        throw std::exception( "RasterizingShadowsComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 3 ] = {};
    deviceContext.CSSetShaderResources( 0, 3, nullResources );

    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 2 ] = {};
    deviceContext.CSSetSamplers( 0, 2, nullSamplers );
}
