#pragma once
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <locale>

#ifdef _MSC_VER
#pragma pack(push, 0x8)
#endif

struct FVector
{
    float                                              X;                                                        // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
    float                                              Y;                                                        // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
    float                                              Z;                                                        // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)

    FVector(float f_x = 0, float f_y = 0, float f_z = 0)
    {
        X = f_x;
        Y = f_y;
        Z = f_z;
    }

    float Dot(const FVector& v) const
    {
        return (X * v.X + Y * v.Y + Z * v.Z);
    }

    FVector operator-(FVector	v) const
    {
        return FVector(X - v.X, Y - v.Y, Z - v.Z);
    }
};

struct FVector2D
{
    float                                              X;                                                        // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
    float                                              Y;                                                        // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)

    inline FVector2D()
        : X(0), Y(0)
    { }

    inline FVector2D(float x, float y)
        : X(x),
        Y(y)
    { }

};

struct FLinearColor
{
    float                                              R;                                                        // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
    float                                              G;                                                        // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
    float                                              B;                                                        // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
    float                                              A;                                                        // 0x000C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)

    inline FLinearColor()
        : R(0), G(0), B(0), A(0)
    { }

    inline FLinearColor(float r, float g, float b, float a)
        : R(r),
        G(g),
        B(b),
        A(a)
    { }

};

//
//struct vector3D
//{
//    float x;
//    float y;
//    float z;
//
//    vector3D(float f_x = 0, float f_y = 0, float f_z = 0)
//    {
//        x = f_x;
//        y = f_y;
//        z = f_z;
//    }
//
//
//    float Dot(const vector3D& v) const
//    {
//        return (x * v.x + y * v.y + z * v.z);
//    }
//
//    vector3D operator-(vector3D	v) const
//    {
//        return vector3D(x - v.x, y - v.y, z - v.z);
//    }
//
//    float	distance(vector3D v)
//    {
//        return sqrt(pow((v.x - x), 2) + pow((v.y - y), 2) + pow((v.z - z), 2));
//    }
//
//    vector3D(float* v)
//    {
//        x = v[0];
//        y = v[1];
//        z = v[2];
//    }
//};



struct vector2D
{
    float x;
    float y;

    vector2D(float f_x = 0, float f_y = 0)
    {
        x = f_x;
        y = f_y;
    }

    float	distance(vector2D v)
    {
        return sqrt(pow((v.x - x), 2) + pow((v.y - y), 2));
    }
};

struct vMatrix
{
    vMatrix() {}
    vMatrix(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23)
    {
        m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
        m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
        m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
    }

    float* operator[](int i)
    {
        /* Assert((i >= 0) && (i < 3));*/
        return m_flMatVal[i];
    }
    const float* operator[](int i) const
    {
        /*Assert((i >= 0) && (i < 3)); */
        return m_flMatVal[i];
    }

    float* Base() { return &m_flMatVal[0][0]; }
    const float* Base() const { return &m_flMatVal[0][0]; }

    float m_flMatVal[3][4];
};

template<class T>
class TArray
{
    friend class FString;

public:
    inline TArray()
    {
        Data = nullptr;
        Count = Max = 0;
    };

    inline int Num() const
    {
        return Count;
    };

    inline T& operator[](int i)
    {
        return Data[i];
    };

    inline const T& operator[](int i) const
    {
        return Data[i];
    };

    inline bool IsValidIndex(int i) const
    {
        return i < Num();
    }

public:
    T* Data;
    int32_t Count;
    int32_t Max;
};

class FString : public TArray<wchar_t>
{
public:
    inline FString()
    {
    };

    FString(const wchar_t* other)
    {
        Max = Count = *other ? std::wcslen(other) + 1 : 0;

        if (Count)
        {
            Data = const_cast<wchar_t*>(other);
        }
    };

    inline bool IsValid() const
    {
        return Data != nullptr;
    }

    inline const wchar_t* c_str() const
    {
        return Data;
    }

    std::string ToString() const
    {
        const auto length = std::wcslen(Data);

        std::string str(length, '\0');

        std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

        return str;
    }
};

class FNameEntry
{
public:
    static const auto NAME_WIDE_MASK = 0x1;
    static const auto NAME_INDEX_SHIFT = 1;

    int32_t Index;
    char UnknownData00[0x04];
    FNameEntry* HashNext;
    union
    {
        char AnsiName[1024];
        wchar_t WideName[1024];
    };

    inline const int32_t GetIndex() const
    {
        return Index >> NAME_INDEX_SHIFT;
    }

    inline bool IsWide() const
    {
        return Index & NAME_WIDE_MASK;
    }

    inline const char* GetAnsiName() const
    {
        return AnsiName;
    }

    inline const wchar_t* GetWideName() const
    {
        return WideName;
    }
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
    inline size_t Num() const
    {
        return NumElements;
    }

    inline bool IsValidIndex(int32_t index) const
    {
        return index < Num() && index >= 0;
    }

    inline ElementType const* const& operator[](int32_t index) const
    {
        return *GetItemPtr(index);
    }

private:
    inline ElementType const* const* GetItemPtr(int32_t Index) const
    {
        const auto ChunkIndex = Index / ElementsPerChunk;
        const auto WithinChunkIndex = Index % ElementsPerChunk;
        const auto Chunk = Chunks[ChunkIndex];
        return Chunk + WithinChunkIndex;
    }

    enum
    {
        ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
    };

    ElementType** Chunks[ChunkTableSize];
    int32_t NumElements;
    int32_t NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;


struct FName
{
    union
    {
        struct
        {
            int32_t ComparisonIndex;
            int32_t Number;
        };

        uint64_t CompositeComparisonValue;
    };

    inline FName()
        : ComparisonIndex(0),
        Number(0)
    {
    };

    inline FName(int32_t i)
        : ComparisonIndex(i),
        Number(0)
    {
    };

    FName(const char* nameToFind)
        : ComparisonIndex(0),
        Number(0)
    {
        static std::unordered_set<int> cache;

        for (auto i : cache)
        {
            if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind))
            {
                ComparisonIndex = i;

                return;
            }
        }

        for (auto i = 0; i < GetGlobalNames().Num(); ++i)
        {
            if (GetGlobalNames()[i] != nullptr)
            {
                if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind))
                {
                    cache.insert(i);

                    ComparisonIndex = i;

                    return;
                }
            }
        }
    };

    static TNameEntryArray *GNames;
    static inline TNameEntryArray& GetGlobalNames()
    {
        return *GNames;
    };

    inline const char* GetName() const
    {
        return GetGlobalNames()[ComparisonIndex]->GetAnsiName();
    };

    inline bool operator==(const FName &other) const
    {
        return ComparisonIndex == other.ComparisonIndex;
    };
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif
