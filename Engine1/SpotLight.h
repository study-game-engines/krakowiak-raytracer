#pragma once

#include "Light.h"

#include <vector>
#include <memory>

#include "Texture2DTypes.h"

#include "uchar4.h"
#include "int2.h"

namespace Engine1
{
    class SpotLight : public Light
    {
        public:

        static const int2 s_shadowMapDimensions;
        static const float s_shadowMapZNear;
        static const float s_shadowMapZFar;

        static std::shared_ptr< SpotLight > createFromFile( const std::string& path );
        static std::shared_ptr< SpotLight > createFromMemory( std::vector< char >::const_iterator& dataIt, const std::vector<char>::const_iterator& dataEndIt );

        SpotLight();
        SpotLight( const SpotLight& light );
        SpotLight( const float3& position, const float3& direction, const float coneAngle, const float3& color = float3( 1.0f, 0.9f, 0.75f ) );
        ~SpotLight();

        void saveToMemory( std::vector< char >& data ) const;
        void saveToFile( const std::string& path ) const;

        virtual Type getType() const;

        void setDirection( const float3& direction );
        void setConeAngle( const float coneAngle );

        void setShadowMap( std::shared_ptr< DepthTexture2D< float > > shadowMap );

        void setInterpolated( const Light& light1, const Light& light2, float ratio ) override;

        float3 getDirection() const;
        float  getConeAngle() const;

        std::shared_ptr< DepthTexture2D< float > >       getShadowMap();
        const std::shared_ptr< DepthTexture2D< float > > getShadowMap() const;

        float44 getShadowMapViewMatrix() const;
        float44 getShadowMapProjectionMatrix() const;

        private:

        float3 m_direction;
        float  m_coneAngle;

        std::shared_ptr< DepthTexture2D< float > > m_shadowMap;
    };
};


