#pragma once
#include "Eigen/Dense"

class Cubemap
{
public:
    enum Face { PosX, NegX, PosY, NegY, PosZ, NegZ };

    explicit Cubemap(int faceSize) : size(faceSize) 
    {
        for (int i = 0; i < 6; ++i) {
            data[i] = new Eigen::Vector4f[size * size];
        }
    }

    ~Cubemap() 
    {
        for (int i = 0; i < 6; ++i) {
            delete[] data[i];
            data[i] = nullptr;
        }
    }

    Eigen::Vector4f* data[6] = { nullptr };
    int size = 0;

    Eigen::Vector4f Sample(const Eigen::Vector3f& direction) const;

    EIGEN_ALWAYS_INLINE int GetSize() const 
    { 
        return size; 
    }

private:
    void DetermineFaceAndUV(const Eigen::Vector3f& dir, Face& face, Eigen::Vector2f& uv) const;
    Eigen::Vector4f SampleFace(Face face, const Eigen::Vector2f& uv) const;
};

