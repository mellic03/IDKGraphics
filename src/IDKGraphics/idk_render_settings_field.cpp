#include "idk_render_settings_field.hpp"

#include <fstream>
#include <libidk/idk_serialize.hpp>


// size_t
// idk::SettingsField::serialize( std::ofstream &stream ) const
// {
//     size_t n = 0;
//     n += idk::streamwrite(stream, m_name);
//     n += idk::streamwrite(stream, m_dtype);
//     n += idk::streamwrite(stream, m_data);
//     return n;
// }


// size_t
// idk::SettingsField::deserialize( std::ifstream &stream )
// {
//     size_t n = 0;
//     n += idk::streamread(stream, m_name);
//     n += idk::streamread(stream, m_dtype);
//     n += idk::streamread(stream, m_data);
//     return n;
// }



size_t
idk::RenderConfig::serialize( std::ofstream &stream ) const
{
    size_t n = 0;

    uint32_t num_groups = this->groups.size();
    n += idk::streamwrite(stream, num_groups);

    for (const auto &[name, group]: this->groups)
    {
        uint32_t num_fields = group.fields.size();
        n += idk::streamwrite(stream, num_fields);
        n += idk::streamwrite(stream, name);

        for (const auto &[field_name, field]: group)
        {
            n += idk::streamwrite(stream, field_name);
            n += idk::streamwrite(stream, field.m_dtype);
            n += idk::streamwrite(stream, field.m_data);
        }
    }

    return n;
}


size_t
idk::RenderConfig::deserialize( std::ifstream &stream )
{
    size_t n = 0;

    this->groups.clear();

    uint32_t num_groups = 0;
    uint32_t num_fields = 0;
    std::string group_name;
    std::string field_name;


    n += idk::streamread(stream, num_groups);

    for (uint32_t i=0; i<num_groups; i++)
    {
        n += idk::streamread(stream, num_fields);
        n += idk::streamread(stream, group_name);

        this->groups[group_name] = SettingsGroup();
        auto &group = this->groups[group_name];

        for (uint32_t j=0; j<num_fields; j++)
        {
            n += idk::streamread(stream, field_name);
            group[field_name] = SettingsField();

            n += idk::streamread(stream, group[field_name].m_dtype);
            n += idk::streamread(stream, group[field_name].m_data);
        }
    }


    return n;
}
