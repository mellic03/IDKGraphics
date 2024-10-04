#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>


namespace idk
{
    class SettingsField;
    class SettingsGroup;
    struct RenderConfig;
}


class idk::SettingsField
{
public:

private:
    friend class idk::RenderConfig;
    std::string m_dtype;
    uint32_t    m_data[16];


public:

    SettingsField()
    :   m_dtype("")
    {

    }

    template <typename T>
    SettingsField( const std::string &dtype, const T &value )
    :   m_dtype(dtype)
    {
        std::memcpy(m_data, &value, sizeof(T));
    }

    // const char *getName() { return m_name.c_str(); }
    const char *getType() { return m_dtype.c_str(); }

    template <typename T>
    T *getValue() { return reinterpret_cast<T *>(&(m_data[0])); }


    // size_t serialize( std::ofstream& ) const;
    // size_t deserialize( std::ifstream& );

};



class idk::SettingsGroup
{
public:
    std::string program;
    std::map<std::string, SettingsField> fields;
    bool changed = true;

    SettingsGroup()
    {

    }

    SettingsGroup( const std::string &program_name, const std::map<std::string, SettingsField> &data )
    :   program(program_name), fields(data)
    {
        
    }


    SettingsField &operator[] (std::string &str) { return fields[str]; }
    const SettingsField &operator[] (const std::string &str) { return fields[str]; }

    auto begin() { return fields.begin(); }
    auto end()   { return fields.end(); }

    const auto begin() const { return fields.begin(); }
    const auto end()   const { return fields.end(); }

};



#define CONFIG_FIELD(dtype, value) idk::SettingsField(#dtype, dtype(value))

struct idk::RenderConfig
{
    SettingsGroup SSAO = SettingsGroup("SSAO", {
        { "enabled",  CONFIG_FIELD( bool,  false  ) },
        { "strength", CONFIG_FIELD( float, 1.0f   ) },
        { "samples",  CONFIG_FIELD( int,   1      ) },
        { "radius",   CONFIG_FIELD( float, 0.125f ) },
        { "bias",     CONFIG_FIELD( float, 0.125f ) }
    });

    SettingsGroup TAA = SettingsGroup("TAA", {
        {"enabled", CONFIG_FIELD( bool,  false )},
        {"factor",  CONFIG_FIELD( int,   1     )}
    });

    SettingsGroup VOL = SettingsGroup("dirlight-volumetric", {
        {"enabled",     CONFIG_FIELD( bool,  false )},
        {"res_divisor", CONFIG_FIELD( int,   4     )},
        {"samples",     CONFIG_FIELD( int,   16    )}
    });

    std::map<std::string, SettingsGroup> groups;
    bool changed = true;

    RenderConfig()
    {
        groups = {
            { "SSAO", SSAO },
            { "TAA", TAA },
            { "Volumetrics", VOL }
        };
    }

    size_t serialize( std::ofstream& ) const;
    size_t deserialize( std::ifstream& );

};


