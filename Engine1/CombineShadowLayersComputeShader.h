#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class CombineShadowLayersComputeShader : public ComputeShader
    {

        public:

        CombineShadowLayersComputeShader();
        virtual ~CombineShadowLayersComputeShader();

        void initialize();

        void setParameters( ID3D11DeviceContext3& deviceContext,
                            Texture2D< unsigned char >& hardShadow,
                            Texture2D< unsigned char >& mediumShadow,
                            Texture2D< unsigned char >& softShadow );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        // Copying is not allowed.
        CombineShadowLayersComputeShader( const CombineShadowLayersComputeShader& ) = delete;
        CombineShadowLayersComputeShader& operator=( const CombineShadowLayersComputeShader& ) = delete;
    };
}









