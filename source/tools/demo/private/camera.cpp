#include "camera.h"

namespace cera
{
    camera::camera()
        : m_ViewDirty(true)
        , m_InverseViewDirty(true)
        , m_ProjectionDirty(true)
        , m_InverseProjectionDirty(true)
        , m_vFoV(45.0f)
        , m_AspectRatio(1.0f)
        , m_zNear(0.1f)
        , m_zFar(100.0f)
    {
        pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
        pData->m_Translation = DirectX::XMVectorZero();
        pData->m_Rotation = DirectX::XMQuaternionIdentity();
    }

    camera::~camera()
    {
        _aligned_free(pData);
    }

    void XM_CALLCONV camera::set_LookAt(DirectX::FXMVECTOR eye, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up)
    {
        pData->m_ViewMatrix = DirectX::XMMatrixLookAtLH(eye, target, up);

        pData->m_Translation = eye;
        pData->m_Rotation = XMQuaternionRotationMatrix(XMMatrixTranspose(pData->m_ViewMatrix));

        m_InverseViewDirty = true;
        m_ViewDirty = false;
    }

    DirectX::XMMATRIX camera::get_ViewMatrix() const
    {
        if (m_ViewDirty)
        {
            UpdateViewMatrix();
        }
        return pData->m_ViewMatrix;
    }

    DirectX::XMMATRIX camera::get_InverseViewMatrix() const
    {
        if (m_InverseViewDirty)
        {
            pData->m_InverseViewMatrix = XMMatrixInverse(nullptr, pData->m_ViewMatrix);
            m_InverseViewDirty = false;
        }

        return pData->m_InverseViewMatrix;
    }

    void camera::set_Projection(float fovy, float aspect, float zNear, float zFar)
    {
        m_vFoV = fovy;
        m_AspectRatio = aspect;
        m_zNear = zNear;
        m_zFar = zFar;

        m_ProjectionDirty = true;
        m_InverseProjectionDirty = true;
    }

    DirectX::XMMATRIX camera::get_ProjectionMatrix() const
    {
        if (m_ProjectionDirty)
        {
            UpdateProjectionMatrix();
        }

        return pData->m_ProjectionMatrix;
    }

    DirectX::XMMATRIX camera::get_InverseProjectionMatrix() const
    {
        if (m_InverseProjectionDirty)
        {
            UpdateInverseProjectionMatrix();
        }

        return pData->m_InverseProjectionMatrix;
    }

    void camera::set_FoV(float fovy)
    {
        if (m_vFoV != fovy)
        {
            m_vFoV = fovy;
            m_ProjectionDirty = true;
            m_InverseProjectionDirty = true;
        }
    }

    float camera::get_FoV() const
    {
        return m_vFoV;
    }


    void XM_CALLCONV camera::set_Translation(DirectX::FXMVECTOR translation)
    {
        pData->m_Translation = translation;
        m_ViewDirty = true;
    }

    DirectX::XMVECTOR camera::get_Translation() const
    {
        return pData->m_Translation;
    }

    void camera::set_Rotation(DirectX::FXMVECTOR rotation)
    {
        pData->m_Rotation = rotation;
    }

    DirectX::XMVECTOR camera::get_Rotation() const
    {
        return pData->m_Rotation;
    }

    void XM_CALLCONV camera::Translate(DirectX::FXMVECTOR translation, Space space)
    {
        switch (space)
        {
        case Space::Local:
        {
            DirectX::operator+=(pData->m_Translation, DirectX::XMVector3Rotate(translation, pData->m_Rotation));
        }
        break;
        case Space::World:
        {
            DirectX::operator+=(pData->m_Translation, translation);
        }
        break;
        }

        pData->m_Translation = DirectX::XMVectorSetW(pData->m_Translation, 1.0f);

        m_ViewDirty = true;
        m_InverseViewDirty = true;
    }

    void camera::Rotate(DirectX::FXMVECTOR quaternion)
    {
        pData->m_Rotation = DirectX::XMQuaternionMultiply(quaternion, pData->m_Rotation);

        m_ViewDirty = true;
        m_InverseViewDirty = true;
    }

    void camera::UpdateViewMatrix() const
    {
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(pData->m_Rotation));
        DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::operator*(-1.0f, pData->m_Translation));

        pData->m_ViewMatrix = translationMatrix * rotationMatrix;

        m_InverseViewDirty = true;
        m_ViewDirty = false;
    }

    void camera::UpdateInverseViewMatrix() const
    {
        if (m_ViewDirty)
        {
            UpdateViewMatrix();
        }

        pData->m_InverseViewMatrix = XMMatrixInverse(nullptr, pData->m_ViewMatrix);
        m_InverseViewDirty = false;
    }

    void camera::UpdateProjectionMatrix() const
    {
        pData->m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_vFoV), m_AspectRatio, m_zNear, m_zFar);

        m_ProjectionDirty = false;
        m_InverseProjectionDirty = true;
    }

    void camera::UpdateInverseProjectionMatrix() const
    {
        if (m_ProjectionDirty)
        {
            UpdateProjectionMatrix();
        }

        pData->m_InverseProjectionMatrix = XMMatrixInverse(nullptr, pData->m_ProjectionMatrix);
        m_InverseProjectionDirty = false;
    }
}